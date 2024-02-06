#include "monetra_api.h"
#include "monetra_conn.h"
#include "monetra_trans.h"

static LM_trans_response_type_t LM_conn_determine_response_type(const unsigned char *msg, size_t msg_len)
{
    size_t i;

    /* - Scan at most 32 bytes or until \n is hit, whichever comes first
     * - If first byte is a "=" or "," message is some other format (e.g. PEM certificate)
     * - If a "=", other than the first byte, is encountered before a "," it is KV data.
     * - If a "," is hit before an "=", it is CSV data
     * - If a newline is hit before 32bytes, assume it is a single-column CSV
     * - If a non-alpha numeric (or quote) char is hit, its blob data
     * - If neither is hit and there is still data on the line, it is blob data
     */
    if (msg_len > 32)
        msg_len = 32;

    if (msg_len == 0 || *msg == '=' || *msg == ',')
        return LM_TRANS_RESPONSE_BULK;

    for (i=1; i<msg_len; i++) {
        switch (msg[i]) {
            case '=':
                return LM_TRANS_RESPONSE_KV;
            case ',':
                return LM_TRANS_RESPONSE_CSV;
            case '\r':
            case '\n':
                return LM_TRANS_RESPONSE_CSV;
            case '"':
            case '_':
            case '-':
                break;
            default:
                if (!M_chr_isalnumsp((char)msg[i]))
                    return LM_TRANS_RESPONSE_BULK;
                break;
        }
    }

    return LM_TRANS_RESPONSE_BULK;
}


static char *M_hash_dict_deserialize_unquote(const char *str, unsigned char quote_char)
{
    size_t len;
    size_t i;
    char  *ret;
    size_t cnt = 0;

    if (M_str_isempty(str))
        return NULL;

    len = M_str_len(str);

    /* remove surrounding quotes */
    if (len >=2 && str[0] == quote_char && str[len-1] == quote_char) {
        str++;
        len-=2;
    }

    ret = M_malloc_zero(len+1);

    /* remove duplicate quotes */
    for (i=0; i<len; i++) {
        ret[cnt++] = str[i];

        /* quotes escape other quotes */
        if (str[i] == quote_char && str[i+1] == quote_char)
            i++;
    }

    return ret;
}


/*! Convert a key/value pair string into a dictionary */
static M_hash_dict_t *LM_hash_dict_deserialize(const char *str, size_t s_len, M_bool trim_whitespace, unsigned char key_delim, unsigned char val_delim, unsigned char quote_char, unsigned char escape_char)
{
    M_hash_dict_t  *dict    = NULL;
    char          **kvs     = NULL;
    size_t          num_kvs = 0;
    char          **kv      = NULL;
    size_t          num_kv  = 0;
    size_t          i;
    M_bool          success = M_FALSE;

    if (str == NULL || s_len == 0) {
        return NULL;
    }

    kvs = M_str_explode_quoted(val_delim, str, s_len, quote_char, escape_char, 0, &num_kvs, NULL);
    if (kvs == NULL) {
        goto cleanup;
    }

    dict = M_hash_dict_create(16, 75, M_HASH_DICT_CASECMP);
    for (i=0; i<num_kvs; i++) {
        char *temp;
        /* Skip blank lines, should really only be the last line */
        if (M_str_isempty(kvs[i]))
            continue;

        kv = M_str_explode_str_quoted(key_delim, kvs[i], quote_char, escape_char, 2, &num_kv);
        if (kv == NULL || num_kv != 2) {
            goto cleanup;
        }

        /* Trim unquoted whitespace (space, tab, CR/LF) from key/value if requested */
        if (trim_whitespace) {
            M_str_trim(kv[0]);
            M_str_trim(kv[1]);
        }

        /* Remove quotes */
        temp = M_hash_dict_deserialize_unquote(kv[1], quote_char);
        M_hash_dict_insert(dict, kv[0], temp);
        M_free(temp);

        M_str_explode_free(kv, num_kv);
        kv = NULL; num_kv = 0;
    }

    success = M_TRUE;

cleanup:
    M_str_explode_free(kvs, num_kvs);
    M_str_explode_free(kv, num_kv);
    if (!success) {
        M_hash_dict_destroy(dict);
        dict = NULL;
    }

    return dict;
}


/* Parse message in format of   <MSGID><FS><DATA>  (no STX/ETX framing).
 * LM_conn_t is already locked when we enter this function.
 * On successful parse, updates the status of the transaction and moves
 * it to the DONE queue in the conn */
static LM_conn_parse_error_t LM_conn_parse_msg(LM_conn_t *conn, LM_trans_t **trans, const unsigned char *msg, size_t msg_len)
{
    M_parser_t           *parser;
    M_uint64              tran_id = 0;
    LM_conn_parse_error_t err     = LM_CONN_PARSE_ERROR_ERROR;
    unsigned char         b;
    const unsigned char  *data    = NULL;
    size_t                data_len;

    parser = M_parser_create_const(msg, msg_len, M_PARSER_FLAG_NONE);

    /* Read transaction id */
    if (!M_parser_read_uint(parser, M_PARSER_INTEGER_ASCII, 0, 10, &tran_id))
        goto end;

    /* Read FS (0x1c) */
    if (!M_parser_read_byte(parser, &b) || b != 0x1c)
        goto end;

    /* Look up transaction */
    *trans = M_hash_u64vp_get_direct(conn->trans_id_map, tran_id);
    if (*trans == NULL || (*trans)->status != LM_TRANS_STATUS_PENDING) {
        /* orphaned transaction, or duplicate (wtf?) */
        err = LM_CONN_PARSE_ERROR_WAIT;
        goto end;
    }


    M_thread_mutex_lock((*trans)->lock);
    data                    = M_parser_peek(parser);
    data_len                = M_parser_len(parser);
    (*trans)->status        = LM_TRANS_STATUS_DONE;
    (*trans)->response_type = LM_conn_determine_response_type(data, data_len);
    if ((*trans)->response_type == LM_TRANS_RESPONSE_KV) {
        (*trans)->response_params = LM_hash_dict_deserialize((const char *)data, data_len, M_TRUE, '=', '\n', '"', '"');
    } else {
        /* Duplicate data into raw since it isn't KV data, but null terminate it because it is expected
         * to be string data */
        (*trans)->response_raw = M_malloc(data_len + 1);
        M_mem_copy((*trans)->response_raw, data, data_len);
        (*trans)->response_raw[data_len] = 0;
    }

    /* Swap queues in conn */
    M_queue_take(conn->trans_pending, *trans);
    M_queue_insert(conn->trans_done, *trans);

    M_thread_mutex_unlock((*trans)->lock);

    /* Successful parse */
    err = LM_CONN_PARSE_ERROR_SUCCESS;

end:
    M_parser_destroy(parser);
    return err;
}

/* LM_conn_t is already locked when passed to this function.
 * Any transaction returned will already be moved to the "DONE" queue, just an
 * event needs to be generated to let the user know this occurred whent his
 * returns. */
LM_conn_parse_error_t LM_conn_parse_trans_buffer(LM_conn_t *conn, LM_trans_t **trans)
{
    unsigned char         b;
    size_t                msg_len;
    const unsigned char  *msg;
    LM_conn_parse_error_t err = LM_CONN_PARSE_ERROR_WAIT;

    /* Loop because an err == WAIT in this case actually means we successfully
     * parsed a transaction *but* it was orphaned (deleted before response came)
     * back. */
    while (err == LM_CONN_PARSE_ERROR_WAIT) {
        if (M_parser_len(conn->inbuf) < 5 /* STX 1byteID FS 1byteMsg ETX */)
            return LM_CONN_PARSE_ERROR_WAIT;

        /* Guaranteed to work */
        M_parser_peek_byte(conn->inbuf, &b);

        /* Malformed */
        if (b != 0x02)
            return LM_CONN_PARSE_ERROR_ERROR;

        /* Mark our position, its time to see if we have an ETX */
        M_parser_mark(conn->inbuf);

        /* Consume until ETX */
        b = 0x03;
        if (M_parser_consume_until(conn->inbuf, &b, 1, M_TRUE) == 0) {
            /* No ETX yet, must not be a complete message, rewind so we can start again */
            M_parser_mark_rewind(conn->inbuf);
            return LM_CONN_PARSE_ERROR_WAIT;
        }

        /* Lets pop the entire message into a new parser.  We want to do this without
         * duplicating data though, so we're going to peek it */
        msg = M_parser_peek_mark(conn->inbuf, &msg_len);
        if (msg == NULL || msg_len < 5) {
            /* don't think this could be possible! */
            return LM_CONN_PARSE_ERROR_ERROR;
        }

        /* Pass without STX/ETX */
        err = LM_conn_parse_msg(conn, trans, msg+1, msg_len-2);

        /* Clear the mark, this will clear the memory of the consumed data */
        M_parser_mark_clear(conn->inbuf);
    }

    return err;
}

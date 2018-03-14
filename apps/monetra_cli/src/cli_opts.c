#include "monetra_cli.h"

const char *DEFAULT_USERNAME   = "loopback";
const char *DEFAULT_PASSWORD   = "test123";
const char *DEFAULT_HOST       = "localhost";
M_uint16    DEFAULT_PORT_USER  = 8665;
M_uint16    DEFAULT_PORT_ADMIN = 8666;

static struct {
	const char *name;
	const char *val;
} CARDS[] = {
	{ "visa",          "trackdata=\"%B4111111111111111^TEST CARD/VI^25121015432112345678?;4111111111111111=25121015432112345678?\"" },
	{ "visa_keyed",    "account=4111111111111111,expdate=1220" },
	{ "visa_icc",      "icc=4F07A0000000031010500B564953412043524544495457134761739001010010D22122011143804400000F5A0847617390010100105F201A554154205553412F5465737420436172642030342020202020205F24032212315F280208405F2A0208405F2D02656E5F300202015F34010182021C008407A0000000031010950580800080009A031701059B0268009C01009F02060000000100009F03060000000000009F0607A00000000310109F0702FF009F0902008C9F100706010A03A020009F1101019F120B56697361204372656469749F1A0208409F1B04000000009F1E0838303338383535389F1F183131343338303434383930303030303030303030303030309F21030933379F2608D737017E33C786929F2701809F3303E0F8C89F34031F00029F3501229F360200E09F3704526F34219F3901059F4005F000F0F0019F4104000000039F530152" },
	{ "mc",            "trackdata=\";5454545454545454=25021015432112345678?\"" },
	{ "mc_keyed",      "account=5454545454545454,expdate=1120" },
	{ "mc_icc",        "icc=4F07A0000000041010500A4D41535445524341524457135413330089604111D22122010123409172029F5A0854133300896041115F201A554154205553412F5465737420436172642030362020202020205F24032212315F280208405F2A0208405F2D02656E5F300202015F340101820218008407A0000000041010950580000080009A031701059B0268009C01009F02060000000110009F03060000000000009F0607A00000000410109F0702FF009F090200029F10120110A00003220000000000000000000000FF9F1101019F120A4D6173746572436172649F1A0208409F1B04000000009F1E0838303338383535389F1F183031323334303931373230303030303030303030303030309F21030937149F2608EEC06DB6F736A06B9F2701809F3303E0F8C89F34035E03009F3501229F360200229F37048DFE9D539F3901059F4005F000F0F0019F4104000000049F420208409F530152" },
	{ "mc2",           "trackdata=\"%B2222333344445553^TEST CARD/MC2^25021015432112345678?;2222333344445553=25021015432112345678?\"" },
	{ "mc2_keyed",     "account=2222333344445553,expdate=1020" },
	{ "amex",          "trackdata=\"%B371449635398431^TEST CARD/AX^25021015432112345678?;371449635398431=25021015432112345678?\"" },
	{ "amex_keyed",    "account=371449635398431,expdate=0920" },
	{ "amex_icc",      "icc=4F08A0000000250104025010414D45524943414E20455850524553535713374245001751006D200120115021234500000F5A08374245001751006F5F201A554154205553412F5465737420436172642031322020202020205F24032001315F280208405F2A0208405F2D02656E5F300202015F34010082021C008408A0000000250104029505800004800099244141443242423133313845453943393246464646303030303030303030303030303034429A031701059B0268009C01009F02060000000130009F03060000000000009F0608A0000000250104029F0702FF009F090200019F100706020103A0B8009F1101019F1210416D65726963616E20457870726573739F1A0208409F1B04000000009F1E0838303338383535389F21030952059F2608A872B84AA7D893319F2701809F3303E0F8C89F34034203009F3501229F360200049F3704F51F1E669F3901059F4005F000F0F0019F4104000000089F420208409F530152" },
	{ "disc",          "trackdata=\"%B6011000995500000^TEST CARD/DI^25021015432112345678?;6011000995500000=25021015432112345678?\"" },
	{ "disc_keyed",    "account=6011000995500000,expdate=0820" },
	{ "disc_icc",      "icc=4F07A00000015230105008444953434F56455257136510000000000216D19122011000074900000F5A0865100000000002165F201A554154205553412F5465737420436172642031362020202020205F24031912315F280208405F2A0208405F2D02656E5F300202015F340101820218008407A0000001523010950580000080009A031701059B0268009C01009F02060000000120009F03060000000000009F0607A00000015230109F0702FF009F090200019F10080106A014090000009F1101019F12064372656469749F1A0208409F1B04000000009F1E0838303338383535389F21030941049F260878FD76F778D1613C9F2701809F3303E0F8C89F34034103029F3501229F360200089F37046B673ECF9F3901059F4005F000F0F0019F4104000000059F530152" },
	{ "jcb",           "trackdata=\"%B3566002020140006^TEST CARD/JC^18021015432112345678?;3566002020140006=18021015432112345678?\"" },
	{ "jcb_keyed",     "account=3566002020140006,expdate=0720" },
	{ "gift",          "trackdata=\"%B6035710110009768^GIFT CARD^240512000000457000?;6035710110009768=240512000000457?\"" },
	{ "gift_keyed",    "account=6035710110009768,expdate=0620" },
	{ "generic",       "trackdata=\"%B9999999800009919^TESTCARD/GENERIC^49127990000000000101?;9999999800009919=49127990000000000101?\"" },
	{ "generic_keyed", "account=9999999800009919,expdate=0520" },
	{ NULL, NULL }
};

static struct {
	const char *name;
	const char *action;
} ACTIONS[] = {
	{ "bt",                 "action=admin,admin=bt"                                                                                                                      },
    { "gl",                 "action=admin,admin=gl"                                                                                                                      },
    { "gut",                "action=admin,admin=gut"                                                                                                                     },
    { "gft",                "action=admin,admin=gft"                                                                                                                     },
    { "recurringlist",      "action=admin,admin=recurringlist"                                                                                                           },

    { "sale",               "action=sale,trackdata=\"%B4111111111111111^TEST CARD/VI^25121015432112345678?;4111111111111111=25121015432112345678?\",amount=1.00,nsf=yes" },
    { "sale_partial",       "action=sale,trackdata=\"%B5454545454545454^TEST CARD/MC^25121015432112345678?;5454545454545454=25121015432112345678?\",amount=6.75,nsf=yes" },
    { "sale_corp",          "action=sale,trackdata=\"%B4055011111111111^TEST CARD/VI^25101015432112345678?;4055011111111111=25101015432112345678?\",amount=2.00"         },

    { "sale_gift",          "action=sale,trackdata=\"%B6035710110009768^GIFT CARD^240512000000457000?;6035710110009768=240512000000457?\",amount=99.00"                  },

    { "sale_debit",         "action=sale,pin=1234567891123456,ksn=6543210987654321,trackdata=\";4788250000028291=25121015432112345601?\",amount=3.00,nsf=yes"            },
    { "sale_debit_partial", "action=sale,pin=1234567891123456,ksn=6543210987654321,trackdata=\";4788250000028291=25121015432112345601?\",amount=6.76,nsf=yes"            },

    { "sale_keyed",         "action=sale,account=4111111111111111,expdate=1225,amount=1.00,zip=99999,nsf=yes"                                                            },
    { "sale_keyed_partial", "action=sale,account=5454545454545454,expdate=1225,amount=6.75,zip=32606,cvv=123,nsf=yes"                                                    },
    { "sale_keyed_gift",    "action=sale,account=6035710110009768,amount=40.00,nsf=yes"                                                                                  },

    { "return",             "action=return,trackdata=%B4111111111111111^TEST CARD/VI^25121015432112345678?,amount=4.00"                                                  },
    { "return_keyed",       "action=return,account=5454545454545454,amount=4.50,expdate=1225,zip=32606"                                                                  },

	/* All EMV Complete transactions need ttid from corresponding sale. */
	{ "sale_emv_visa",         "action=sale,amount=100.00,nsf=yes,serialnum=80388558,device_kernver=0526,"
		"icc=4F07A0000000031010500B564953412043524544495457134761739001010010D22122011143804400000F5A0847617390010100105F201A554154205553412F5465737420436172642030342020202020205F24032212315F280208405F2A0208405F2D02656E5F300202015F34010182021C008407A0000000031010950580800080009A031701059B0268009C01009F02060000000100009F03060000000000009F0607A00000000310109F0702FF009F0902008C9F100706010A03A020009F1101019F120B56697361204372656469749F1A0208409F1B04000000009F1E0838303338383535389F1F183131343338303434383930303030303030303030303030309F21030933379F2608D737017E33C786929F2701809F3303E0F8C89F34031F00029F3501229F360200E09F3704526F34219F3901059F4005F000F0F0019F4104000000039F530152" },
	{ "sale_emv_visa_complete", "action=emvcomplete,"
		"icc=4F07A0000000031010500B564953412043524544495457134761739001010010D22122011143804400000F5F280208405F2A0208405F34010182021C008407A00000000310108A0230308E0A00000000000000001F00910A31601013A1374EE13030950580800080009A031701059B0278009C01009F02060000000100009F03060000000000009F0607A00000000310109F0702FF009F080200969F0902008C9F0D05B0508088009F0E0500000000009F0F05B0508098009F100706010A036020009F1101019F120B56697361204372656469749F1A0208409F1B04000000009F1E0838303338383535389F1F183131343338303434383930303030303030303030303030309F21030933379F26080805208B71AF91CB9F2701409F3303E0F8C89F34031F00029F3501229F360200E09F3704526F34219F3901059F4005F000F0F0019F4104000000039F530152" },
	{ "sale_emv_mc",            "action=sale,amount=110.00,nsf=yes,serialnum=80388558,device_kernver=0526,"
		"icc=4F07A0000000041010500A4D41535445524341524457135413330089604111D22122010123409172029F5A0854133300896041115F201A554154205553412F5465737420436172642030362020202020205F24032212315F280208405F2A0208405F2D02656E5F300202015F340101820218008407A0000000041010950580000080009A031701059B0268009C01009F02060000000110009F03060000000000009F0607A00000000410109F0702FF009F090200029F10120110A00003220000000000000000000000FF9F1101019F120A4D6173746572436172649F1A0208409F1B04000000009F1E0838303338383535389F1F183031323334303931373230303030303030303030303030309F21030937149F2608EEC06DB6F736A06B9F2701809F3303E0F8C89F34035E03009F3501229F360200229F37048DFE9D539F3901059F4005F000F0F0019F4104000000049F420208409F530152" },
	{ "sale_emv_mc_complete",   "action=emvcomplete,"
		"icc=4F07A0000000041010500A4D41535445524341524457135413330089604111D22122010123409172029F5F280208405F2A0208405F340101820218008407A00000000410108A0230308E0E000000000000000042015E031F03950580000080009A031701059B0268009C01009F02060000000110009F03060000000000009F0607A00000000410109F0702FF009F080200029F090200029F0D05B0509C80009F0E0500000000009F0F05B0709C98009F10120110600003220000000000000000000000FF9F1101019F120A4D6173746572436172649F1A0208409F1B04000000009F1E0838303338383535389F1F183031323334303931373230303030303030303030303030309F21030937149F2608936A5847ED6CEA6B9F2701409F3303E0F8C89F34035E03009F3501229F360200229F37048DFE9D539F3901059F4005F000F0F0019F4104000000049F420208409F530152" },
	{ "sale_emv_disc",          "action=sale,amount=120.00,nsf=yes,serialnum=80388558,device_kernver=0526,"
		"icc=4F07A00000015230105008444953434F56455257136510000000000216D19122011000074900000F5A0865100000000002165F201A554154205553412F5465737420436172642031362020202020205F24031912315F280208405F2A0208405F2D02656E5F300202015F340101820218008407A0000001523010950580000080009A031701059B0268009C01009F02060000000120009F03060000000000009F0607A00000015230109F0702FF009F090200019F10080106A014090000009F1101019F12064372656469749F1A0208409F1B04000000009F1E0838303338383535389F21030941049F260878FD76F778D1613C9F2701809F3303E0F8C89F34034103029F3501229F360200089F37046B673ECF9F3901059F4005F000F0F0019F4104000000059F530152" },
	{ "sale_emv_disc_complete", "action=emvcomplete,"
		"icc=4F07A00000015230105008444953434F56455257136510000000000216D19122011000074900000F5F280208405F2A0208405F340101820218008407A00000015230108A0230308E0E0000000000000000420141031F03950580000080009A031701059B0268009C01009F02060000000120009F03060000000000009F0607A00000015230109F0702FF009F080200019F090200019F0D053040E428009F0E0500100000409F0F053068C4F8009F100801066014090000009F1101019F12064372656469749F1701099F1A0208409F1B04000000009F1E0838303338383535389F21030941049F26085A1AB89B5346A7B39F2701409F3303E0F8C89F34034103029F3501229F360200089F37046B673ECF9F3901059F4005F000F0F0019F4104000000059F530152" },
	{ "sale_emv_disc_reversal", "action=reversal,reversal_reason=CARDDECLINE,serialnum=80388558,device_kernver=0526,"
		"icc=4F07A00000015230105008444953434F56455257136510000000000216D19122011000074900000F5F280208405F2A0208405F340101820218008407A00000015230108A0230308E0E0000000000000000420141031F03910AFFFFFFFFFFFFFFFFFFFF950580000080009A031701059B0268009C01009F02060000000009059F03060000000000009F0607A00000015230109F0702FF009F080200019F090200019F0D053040E428009F0E0500100000409F0F053068C4F8009F100801062014096000009F1101019F12064372656469749F1701099F1A0208409F1B04000000009F1E0838303338383535389F21030943569F260886EC57E7357319F19F2701009F3303E0F8C89F34034103029F3501229F3602000A9F3704CC85E77D9F3901059F4005F000F0F0019F4104000000079F530152" },
	{ "sale_emv_amex",          "action=sale,amount=130.00,nsf=yes,ordernum=2011,serialnum=80388558,device_kernver=0526,"
		"icc=4F08A0000000250104025010414D45524943414E20455850524553535713374245001751006D200120115021234500000F5A08374245001751006F5F201A554154205553412F5465737420436172642031322020202020205F24032001315F280208405F2A0208405F2D02656E5F300202015F34010082021C008408A0000000250104029505800004800099244141443242423133313845453943393246464646303030303030303030303030303034429A031701059B0268009C01009F02060000000130009F03060000000000009F0608A0000000250104029F0702FF009F090200019F100706020103A0B8009F1101019F1210416D65726963616E20457870726573739F1A0208409F1B04000000009F1E0838303338383535389F21030952059F2608A872B84AA7D893319F2701809F3303E0F8C89F34034203009F3501229F360200049F3704F51F1E669F3901059F4005F000F0F0019F4104000000089F420208409F530152" },
	{ "sale_emv_amex_complete", "action=emvcomplete,"
		"icc=4F08A0000000250104025010414D45524943414E20455850524553535713374245001751006D200120115021234500000F5F280208405F2A0208405F34010082021C008408A0000000250104028A0230308E100000000000000000420142035E035F03910A232730100C0571103030950580000480009A031701059B0278009C01009F02060000000130009F03060000000000009F0608A0000000250104029F0702FF009F080200019F090200019F0D05B050DC98009F0E0500000000009F0F05B078DC98009F10070602010360B8009F1101019F1210416D65726963616E20457870726573739F1A0208409F1B04000000009F1E0838303338383535389F21030952059F2608F5DBEF1AEE749CD69F2701409F3303E0F8C89F34034203009F3501229F360200049F3704F51F1E669F3901059F4005F000F0F0019F4104000000089F420208409F530152" },
	{ "sale_emv_dna",           "action=sale,amount=140.00,nsf=yes,serialnum=80388558,device_kernver=0526,"
		"icc=4F07A00000062006205003444E4157134000000000000002D19122011000074900000F5A0840000000000000025F201A554154205553412F5445535420434152442031382020202020205F24031912315F280208405F2A0208405F2D02656E5F300202015F340101820218008407A00000062006209505800004800099244442444132314236453246423933434546464646303030303030303030303030303034439A031701059B0268009C01009F02060000000140009F03060000000000009F0607A00000062006209F0702FF009F090200019F10080106A000034000009F1101019F120544656269749F1A0208409F1B04000000009F1E0838303338383535389F21030953399F26081661E82FC5351B719F2701809F3303E0F8C89F34030203009F3501229F360200599F3704A44C049C9F3901059F4005F000F0F0019F4104000000099F530152" },
	{ "sale_emv_dna_complete",  "action=emvcomplete,"
		"icc=4F07A00000062006205003444E4157134000000000000002D19122011000074900000F5F280208405F2A0208405F340101820218008407A00000062006208A0230308E1000000000000000000201020402031F03950580000480009A031701059B0268009C01009F02060000000140009F03060000000000009F0607A00000062006209F0702FF009F080200019F090200019F0D05F040E428009F0E0500100000409F0F05F068FCF8009F100801066000034000009F1101019F120544656269749F1A0208409F1B04000000009F1E0838303338383535389F21030953399F2608F832980FCB3358A19F2701409F3303E0F8C89F34030203009F3501229F360200599F3704A44C049C9F3901059F4005F000F0F0019F4104000000099F530152" },

	{ "return_emv_visa",         "action=return,amount=200.00,serialnum=80388558,device_kernver=0526,"
		"icc=4F07A0000000031010500B564953412043524544495457134761739001010119D22122011143804400000F5A0847617390010101195F201A554154205553412F5465737420436172642030332020202020205F24032212315F280208405F2D02656E5F300202015F3401018407A00000000310109F0702FF009F1101019F120B56697361204372656469749F1A0208409F1B04000000009F1E0838303338383535389F1F183131343338303434303030303030303030303030303030309F390105" },
	{ "return_emv_dna",          "action=return,amount=210.00,serialnum=80388558,device_kernver=0526,"
		"icc=4F07A00000062006205003444E4157134000000000000002D19122011000074900000F5A0840000000000000025F201A554154205553412F5445535420434152442031382020202020205F24031912315F280208405F2A0208405F2D02656E5F300202015F340101820218008407A00000062006209505800004800099243338423841373030433041343841413446464646303030303030303030303030303034449A031701059B0268009C01209F02060000000210009F03060000000000009F0607A00000062006209F0702FF009F090200019F10080106A000034000009F1101019F120544656269749F1A0208409F1B04000000009F1E0838303338383535389F21030954419F260806151098D7A9230A9F2701809F3303E0F8C89F34030203009F3501229F3602005A9F3704B5FD9A709F3901059F4005F000F0F0019F4104000000109F530152" },
	{ "return_emv_dna_complete", "action=emvcomplete,"
		"icc=4F07A00000062006205003444E4157134000000000000002D19122011000074900000F5F280208405F2A0208405F340101820218008407A00000062006208A0230308E1000000000000000000201020402031F03950580000480009A031701059B0268009C01209F02060000000210009F03060000000000009F0607A00000062006209F0702FF009F080200019F090200019F0D05F040E428009F0E0500100000409F0F05F068FCF8009F100801066000034000009F1101019F120544656269749F1A0208409F1B04000000009F1E0838303338383535389F21030954419F2608FDE63CEFE17536BE9F2701409F3303E0F8C89F34030203009F3501229F3602005A9F3704B5FD9A709F3901059F4005F000F0F0019F4104000000109F530152" },

	{ "sale_emv_cless_amex", "action=sale,amount=300.00,rfid=yes,nsf=yes,ordernum=7490000001,serialnum=80388558,device_kernver=0526,"
		"icc=5010414D45524943414E20455850524553535713374245001751006D200170215021234500000F5A08374245001751006F5F201A554154205553412F5465737420436172642031322020202020205F24032001315F2A0208405F2D02656E5F300202015F34010082021C808408A000000025010402950580000080009A031701059B0268009C01009F02060000000300009F03060000000000009F0606A000000025019F07023D009F090200019F100706020103A020009F1210416D65726963616E20457870726573739F1A0208409F1B04000000009F1E0838303338383535389F21030958019F2608A194047D43CC50CC9F2701809F3303E068C89F34035E03009F3501229F360200059F3704E25319DE9F3901079F4005F000F0F0019F4104000000059F6E04D8B04000" },
	{ "sale_emv_cless_mc",   "action=sale,amount=310.00,rfid=yes,nsf=yes,serialnum=80388558,device_kernver=0526,"
		"icc=500A4D41535445524341524457135413330089604111D22122010123409172029F5A0854133300896041115F24032212315F2A0208405F2D02656E5F340101820219808407A0000000041010950500000080009A031701059C01009F02060000000310009F03060000000000009F0702FF009F090200029F10120111A04009220000000000000000000000FF9F1101019F120A4D6173746572436172649F1A0208409F1E0838303338383535389F21030959299F2608C988F5EE999922D79F2701809F3303E060089F34035E03009F3501229F360200049F37044A7275FB9F3901079F4005F000F0F0019F4104000000069F5301529F6E200840000030300000000000000000000000000000000000000000000000000000" },
	{ "sale_emv_cless_visa", "action=sale,amount=320.00,rfid=yes,nsf=yes,serialnum=80388558,device_kernver=0526,"
		"icc=500B564953412043524544495457134761739001010119D22122011143804400000F5A0847617390010101195F24032212315F2A0208405F2D02656E5F340101820220008407A0000000031010950500000000009A031705159C01009F02060000000320009F03060000000000009F0607A00000000310109F100706010A03A000009F1A0208409F1E0838303338383535389F21030814549F26089F40A8DEC8D343349F2701809F3303E0F8C89F34031E03009F3501229F360200369F3704F3619C609F3901079F4005F000F0F0019F4104000000029F6604B7C040009F6C0278009F6E0420700000" },

	{ "return_emv_cless_disc", "action=return,amount=400.00,rfid=yes,serialnum=80388558,device_kernver=0526,"
		"icc=500B444953434F56455220434C57136510000000000216D19122011000074900000F5A0865100000000002165F24031912315F2A0208405F2D02656E5F300202015F340101820218008407A00000015230109505800004800099244333383937334342394330444646323546464646303030303030303030303030303034459A031701059B0248009C01209F02060000000400009F03060000000000009F0702FF009F090200019F100A010F20B080100000B8009F1101019F120B446973636F76657220434C9F1A0208409F1E0838303338383535389F21031000329F26089DA7FEF1B0C4A9E29F2701809F3303E068C89F34030203009F3501229F3602000B9F370434AB10EF9F3901079F4005F000F0F0019F410400000008" },

	{ NULL, NULL }
};

/* Parse a KVS string and load it into a dict. */
static void cli_opts_load_kvs(M_hash_dict_t *kvs, const char *string)
{
	char   **kvs_parts;
	char   **kv_parts;
	char    *temp1;
	char    *temp2;
	size_t   num_kvs = 0;
	size_t   num_kv  = 0;
	size_t   i;

	kvs_parts = M_str_explode_quoted(',', string, M_str_len(string), '"', '\\', 0, &num_kvs, NULL);
	if (kvs_parts == NULL || num_kvs == 0) {
		M_str_explode_free(kvs_parts, num_kvs);
		return;
	}

	/* There is trimming around the kvs but only up to a quote if one is present. We
 	 * want to remove leading an trailing white space but it is legitimate to have it
	 * within a quoted key and value. */
	for (i=0; i<num_kvs; i++) {
		temp1 = M_strdup_unquote(kvs_parts[i], '"', '\\');

		kv_parts = M_str_explode_quoted('=', temp1, M_str_len(temp1), '"', '\\', 0, &num_kv, NULL);
		M_free(temp1);

		if (kv_parts == NULL) {
			M_str_explode_free(kv_parts, num_kv);
			break;
		}
		if (num_kv != 2) {
			M_str_explode_free(kv_parts, num_kv);
			continue;
		}

		temp1 = M_strdup_unquote(kv_parts[0], '"', '\\');
		temp2 = M_strdup_unquote(kv_parts[1], '"', '\\');

		M_hash_dict_insert(kvs, temp1, temp2);

		M_free(temp2);
		M_free(temp1);
		M_str_explode_free(kv_parts, num_kv);
	}

	M_str_explode_free(kvs_parts, num_kvs);
}

static void cli_opts_load_rkeys(M_list_str_t *keys, const char *string)
{
	char   **parts;
	size_t   num_parts = 0;
	size_t   i;

	parts = M_str_explode_quoted(',', string, M_str_len(string), '"', '\\', 0, &num_parts, NULL);
	if (parts == NULL || num_parts == 0) {
		M_str_explode_free(parts, num_parts);
		return;
	}

	for (i=0; i<num_parts; i++) {
		M_list_str_insert(keys, parts[i]);
	}

	M_str_explode_free(parts, num_parts);
}

static void cli_opts_load_pinksn(M_hash_dict_t *kvs, M_bool send)
{
	if (!send)
		return;
	M_hash_dict_insert(kvs, "pin", "1234567890123456");
	M_hash_dict_insert(kvs, "ksn", "6543210987654321");
}

static M_bool cli_opts_merge_kvs(cli_opts_t *opts)
{
	if (M_str_isempty(opts->action_kvs) && M_hash_dict_num_keys(opts->man_kvs) == 0)
		return M_FALSE;

	cli_opts_load_kvs(opts->kvs, opts->action_kvs);
	cli_opts_load_pinksn(opts->kvs, opts->send_pinksn);
	M_hash_dict_merge(&opts->kvs, M_hash_dict_duplicate(opts->card));
	M_hash_dict_merge(&opts->kvs, M_hash_dict_duplicate(opts->man_kvs));
	return M_TRUE;
}

static M_bool cli_parse_random_amount(cli_opts_t *opts, const char *string)
{
	M_parser_t    *parser;
	M_decimal_t    min;
	M_decimal_t    max;
	unsigned char  byte;

	/* Format: [min:]max */
	parser = M_parser_create_const((const unsigned char *)string, M_str_len(string), M_PARSER_FLAG_NONE);

	/* Read min and or max if min is not specified. */
	if (M_parser_read_decimal(parser, 0, M_FALSE, &min) != M_DECIMAL_SUCCESS) {
		M_parser_destroy(parser);
		return M_FALSE;
	}

	/* Was only one value which is the max specified. */
	if (M_parser_len(parser) == 0) {
		opts->min_amount = 1;
		opts->max_amount = (M_uint64)M_decimal_to_int(&min, 2);
		M_parser_destroy(parser);
		return M_TRUE;
	}

	/* Proper format. */
	if (!M_parser_read_byte(parser, &byte) || byte != ':') {
		M_parser_destroy(parser);
		return M_FALSE;
	}

	/* Read max */
	if (M_parser_read_decimal(parser, 0, M_FALSE, &max) != M_DECIMAL_SUCCESS) {
		M_parser_destroy(parser);
		return M_FALSE;
	}

	/* Convert to implied decimal scalar. */
	opts->min_amount = (M_uint64)M_decimal_to_int(&min, 2);
	/* When using M_rand_range (we will) the maximum is not inclusive so
 	 * we need to add 1 cent to it. Otherwise the user won't get the actual
	 * range they've requested. */
	opts->max_amount = (M_uint64)M_decimal_to_int(&max, 2)+1;

	/* Max can't be larger than min. */
	if (opts->min_amount > opts->max_amount) {
		M_parser_destroy(parser);
		return M_FALSE;
	}

	/* Can't have only 8.xx amounts. */
	if (opts->min_amount >= 800 && opts->max_amount <= 899) {
		M_parser_destroy(parser);
		return M_FALSE;
	}

	/* Avoid 8.xx amounts. */
	if (opts->min_amount >= 800 && opts->min_amount <= 899)
		opts->min_amount = 900;
	if (opts->max_amount >= 800 && opts->max_amount <= 899)
		opts->max_amount = opts->min_amount + 1;

	M_parser_destroy(parser);
	return M_TRUE;
}

static char *cli_opts_gen_help(M_getopt_t *g, M_getopt_error_t ret, const char *fail, const char *argv0, M_bool reqhelp)
{
	M_buf_t *buf;
	char    *help;
	size_t   i;

	buf = M_buf_create();

	/* If this wasn't an explicit request for help we're here because of an error. */
	if (!reqhelp) {
		switch (ret) {
			case M_GETOPT_ERROR_INVALIDOPT:
				M_buf_add_str(buf, "Invalid option -- ");
				break;
			case M_GETOPT_ERROR_INVALIDDATATYPE:
				M_buf_add_str(buf, "Invalid option value -- ");
				break;
			case M_GETOPT_ERROR_MISSINGVALUE:
				M_buf_add_str(buf, "Invalid option value missing -- ");
				break;
			case M_GETOPT_ERROR_NONOPTION:
				M_buf_add_str(buf, "Invalid action -- ");
				break;
			default:
				M_buf_add_str(buf, "Error -- ");
				break;
		}
		M_buf_add_str(buf, fail);
		M_buf_add_str(buf, "\n\n");
	}

	M_buf_add_str(buf, "Usage: ");
	M_buf_add_str(buf, argv0);
	M_buf_add_str(buf, " [OPTIONS] [ACTION]");
	M_buf_add_byte(buf, '\n');
	M_buf_add_str(buf, "Run transactions against a Monetra server.");
	M_buf_add_str(buf, "\n\n");

	M_buf_add_str(buf, "KVS can be specified and used in place of or to augment ACTIONS.\n");
	M_buf_add_str(buf, "KVS values will be used instead of ACTION values when a key appears in both.\n");
	M_buf_add_str(buf, "The -r, --random_amount option will override an amount value if provided as a KVS.\n");
	M_buf_add_str(buf, "If multiple -k, --kvs options are specified they will be merged with later values overriding earlier ones.\n");
	M_buf_add_byte(buf, '\n');
	M_buf_add_str(buf, "If -t, --port is not provided the port used will be based on the provided user name. \n");
	M_buf_add_str(buf, "MADMIN or MADMIN sub user names will use the admin port. Otherwise the user port will be used.\n");
	M_buf_add_str(buf, "\n\n");

	M_buf_add_str(buf, "OPTIONS:");
	M_buf_add_byte(buf, '\n');
	help = M_getopt_help(g);
	M_buf_add_str(buf, help);
	M_free(help);
	M_buf_add_byte(buf, '\n');

	M_buf_add_str(buf, "ACTIONS:");
	M_buf_add_byte(buf, '\n');
	for (i=0; ACTIONS[i].name!=NULL; i++) {
		M_buf_add_byte(buf, '\t');
		M_buf_add_str(buf, ACTIONS[i].name);
		M_buf_add_byte(buf, '\n');
	}

	M_buf_add_str(buf, "TEST CARDS:");
	M_buf_add_byte(buf, '\n');
	for (i=0; CARDS[i].name!=NULL; i++) {
		M_buf_add_byte(buf, '\t');
		M_buf_add_str(buf, CARDS[i].name);
		M_buf_add_byte(buf, '\n');
	}

	M_buf_add_byte(buf, '\n');
	M_buf_add_str(buf, "DEFAULTS:");
	M_buf_add_byte(buf, '\n');
	M_buf_add_str(buf, "\tUsername:   ");
	M_buf_add_str(buf, DEFAULT_USERNAME);
	M_buf_add_byte(buf, '\n');
	M_buf_add_str(buf, "\tPassword:   ");
	M_buf_add_str(buf, DEFAULT_PASSWORD);
	M_buf_add_byte(buf, '\n');
	M_buf_add_str(buf, "\tHost:       ");
	M_buf_add_str(buf, DEFAULT_HOST);
	M_buf_add_byte(buf, '\n');
	M_buf_add_str(buf, "\tUser port:  ");
	M_buf_add_uint(buf, DEFAULT_PORT_USER);
	M_buf_add_byte(buf, '\n');
	M_buf_add_str(buf, "\tAdmin port: ");
	M_buf_add_uint(buf, DEFAULT_PORT_ADMIN);
	M_buf_add_byte(buf, '\n');
	M_buf_add_str(buf, "\tCert validation: ");
	M_buf_add_str(buf, "none\n");
	M_buf_add_str(buf, "\t\tChoices: none, cert_only, cert_fuzzy, full");
	M_buf_add_str(buf, "\n\n");

	M_buf_add_str(buf, "EXAMPLE:");
	M_buf_add_byte(buf, '\n');
	M_buf_add_byte(buf, '\t');
	M_buf_add_str(buf, argv0);
	M_buf_add_str(buf, " -d 3 -r 4.50:10.03 sale");
	M_buf_add_byte(buf, '\n');
	M_buf_add_byte(buf, '\t');
	M_buf_add_str(buf, argv0);
	M_buf_add_str(buf, " --username MADMIN --password password --kvs action=chngpwd,pwd=test123");
	M_buf_add_byte(buf, '\n');

	return M_buf_finish_str(buf, NULL);
}

static M_bool cli_nonopt_cb(size_t idx, const char *option, void *thunk)
{
	cli_opts_t *opts = thunk;
	size_t      i;

	(void)idx;

	/* Figure out which action was requested and if it's valid. */
	for (i=0; ACTIONS[i].name!=NULL; i++) {
		if (M_str_caseeq(option, ACTIONS[i].name)) {
			M_free(opts->action_kvs);
			opts->action_kvs = M_strdup(ACTIONS[i].action);
			return M_TRUE;
		}
	}

	return M_FALSE;
}

static M_bool cli_integer_cb(char short_opt, const char *long_opt, M_int64 *integer, void *thunk)
{
	cli_opts_t *opts = thunk;
	M_uint64    val;

	(void)short_opt;

	if (integer == NULL || *integer < 0)
		return M_FALSE;
	val = (M_uint64)*integer;

	if (M_str_caseeq(long_opt, "port")) {
		opts->port     = (M_uint16)val;
		opts->port_set = M_TRUE;
	} else if (M_str_caseeq(long_opt, "dup")) {
		opts->dup = val;
	} else {
		return M_FALSE;
	}

	return M_TRUE;
}

static M_bool cli_string_cb(char short_opt, const char *long_opt, const char *string, void *thunk)
{
	cli_opts_t *opts = thunk;
	size_t      i;

	(void)short_opt;

	if (M_str_caseeq(long_opt, "username")) {
		M_free(opts->username);
		opts->username = M_strdup(string);
		return M_TRUE;
	} else if (M_str_caseeq(long_opt, "password")) {
		M_free(opts->password);
		opts->password = M_strdup(string);
		return M_TRUE;
	}

	if (M_str_isempty(string))
		return M_FALSE;

	if (M_str_caseeq(long_opt, "host")) {
		M_free(opts->host);
		opts->host = M_strdup(string);
	} else if (M_str_caseeq(long_opt, "keyfile")) {
		M_free(opts->keyfile);
		opts->keyfile = M_strdup(string);
	} else if (M_str_caseeq(long_opt, "certfile")) {
		M_free(opts->certfile);
		opts->certfile = M_strdup(string);
	} else if (M_str_caseeq(long_opt, "cadir")) {
		M_free(opts->cadir);
		opts->cadir = M_strdup(string);
	} else if (M_str_caseeq(long_opt, "cert_validation")) {
		/* Verify the validation level string. While not advertised in
 		 * help the cert_ ones will be accepted with the prefix. */
		if (M_str_caseeq(string, "none")) {
			opts->certvalidation = M_TLS_VERIFY_NONE;
		} else if (M_str_caseeq(string, "cert_only") || M_str_caseeq(string, "only")) {
			opts->certvalidation = M_TLS_VERIFY_CERT_ONLY;
		} else if (M_str_caseeq(string, "cert_fuzzy") || M_str_caseeq(string, "fuzzy")) {
			opts->certvalidation = M_TLS_VERIFY_CERT_FUZZY;
		} else if (M_str_caseeq(string, "full")) {
			opts->certvalidation = M_TLS_VERIFY_FULL;
		} else {
			return M_FALSE;
		}
	} else if (M_str_caseeq(long_opt, "kvs")) {
		cli_opts_load_kvs(opts->man_kvs, string);
	} else if (M_str_caseeq(long_opt, "rkeys")) {
		cli_opts_load_rkeys(opts->remove_keys, string);
	} else if (M_str_caseeq(long_opt, "card")) {
		for (i=0; CARDS[i].name!=NULL; i++) {
			if (M_str_caseeq(string, CARDS[i].name)) {
				cli_opts_load_kvs(opts->card, CARDS[i].val);
				return M_TRUE;
			}
		}
		return M_FALSE;
	} else if (M_str_caseeq(long_opt, "actions")) {
		opts->help = M_TRUE;
	} else if (M_str_caseeq(long_opt, "random_amount")) {
		return cli_parse_random_amount(opts, string);
	} else {
		return M_FALSE;
	}

	return M_TRUE;
}

static M_bool cli_bool_cb(char short_opt, const char *long_opt, M_bool boolean, void *thunk)
{
	cli_opts_t *opts = thunk;

	(void)short_opt;

	if (M_str_caseeq(long_opt, "help")) {
		opts->help = boolean;
	} else if (M_str_caseeq(long_opt, "serial")) {
		opts->send_serial = M_TRUE;
	} else if (M_str_caseeq(long_opt, "pin-ksn")) {
		opts->send_pinksn = M_TRUE;
	} else {
		return M_FALSE;
	}

	return M_TRUE;
}

cli_opts_t *cli_opts_create(void)
{
	cli_opts_t *opts;

	opts = M_malloc_zero(sizeof(*opts));

	opts->username       = M_strdup(DEFAULT_USERNAME);
	opts->password       = M_strdup(DEFAULT_PASSWORD);
	opts->host           = M_strdup(DEFAULT_HOST);
	opts->port           = DEFAULT_PORT_USER;
	opts->certvalidation = M_TLS_VERIFY_NONE;
	opts->man_kvs        = M_hash_dict_create(8, 75, M_HASH_DICT_CASECMP);
	opts->card           = M_hash_dict_create(8, 75, M_HASH_DICT_CASECMP);
	opts->kvs            = M_hash_dict_create(8, 75, M_HASH_DICT_CASECMP|M_HASH_DICT_KEYS_ORDERED|M_HASH_DICT_KEYS_SORTASC);
	opts->remove_keys    = M_list_str_create(M_LIST_STR_NONE);
	opts->dup            = 1;

	return opts;
}

void cli_opts_destroy(cli_opts_t *opts)
{
	if (opts == NULL)
		return;

	M_free(opts->username);
	M_free(opts->password);
	M_free(opts->host);
	M_free(opts->keyfile);
	M_free(opts->certfile);
	M_free(opts->cadir);
	M_list_str_destroy(opts->remove_keys);
	M_hash_dict_destroy(opts->man_kvs);
	M_hash_dict_destroy(opts->kvs);
	M_hash_dict_destroy(opts->card);
	M_free(opts->action_kvs);

	M_free(opts);
}

cli_trans_t *cli_parse_args(int argc, const char *const *argv)
{
	M_getopt_t       *g;
	cli_opts_t       *opts;
	cli_trans_t      *trans = NULL;
	char             *help;
	const char       *fail = "?";
	M_getopt_error_t  ret;

	opts = cli_opts_create();
	g    = M_getopt_create(cli_nonopt_cb);

	M_getopt_addstring(g, 'u', "username", M_TRUE, "Username to run the transaction as.", cli_string_cb);
	M_getopt_addstring(g, 'p', "password", M_TRUE, "Password to use.", cli_string_cb);
	M_getopt_addstring(g, 'a', "host", M_TRUE, "Host name of the server.", cli_string_cb);
	M_getopt_addinteger(g, 't', "port", M_TRUE, "Port to connet to.", cli_integer_cb);
	M_getopt_addstring(g, 0, "keyfile", M_TRUE, "Key file to use when certificate restrictions are in place.", cli_string_cb);
	M_getopt_addstring(g, 0, "certfile", M_TRUE, "Cert file to use when certificate restrictions are in place.", cli_string_cb);
	M_getopt_addstring(g, 0, "cadir", M_TRUE, "Directory with PEM encoded CA root certificates that will be loaded.", cli_string_cb);
	M_getopt_addstring(g, 0, "cert_validation", M_TRUE, "Level of SSL certificate validation.", cli_string_cb);
	M_getopt_addstring(g, 'k', "kvs", M_TRUE, "Key vaule pairs separated by , to run. E.G. key=val,key=val.", cli_string_cb);
	M_getopt_addstring(g, 'l', "rkeys", M_TRUE, "Keys that should not be sent. May be present from an action. E.G. key,key.", cli_string_cb);
	M_getopt_addstring(g, 'c', "card", M_TRUE, "Test card to use. See help for choices", cli_string_cb);
	M_getopt_addinteger(g, 'd', "dup", M_FALSE, "Number of times the transaction should be duplicated and run in one request. Default is 1.", cli_integer_cb);
	M_getopt_addstring(g, 'r', "random_amount", M_TRUE, "Use a random amount [min:]max (will never be $8.00 - $8.99 due to delay trigger). Default min if not specified is $0.01.", cli_string_cb);
	M_getopt_addboolean(g, 's', "serial", M_FALSE, "Send transaction one at a time. Default is to send them in parallel when using the dup option.", cli_bool_cb);
	M_getopt_addboolean(g, 0, "pin-ksn", M_FALSE, "Insert generic PIN/KSN", cli_bool_cb);
	M_getopt_addboolean(g, 'h', "help", M_FALSE, "Help", cli_bool_cb);

	ret = M_getopt_parse(g, argv, argc, &fail, opts);
	if (ret == M_GETOPT_ERROR_SUCCESS && !opts->help) {
		/* Merge the action, manual, and card kvs. */
		cli_opts_merge_kvs(opts);
		trans = cli_trans_create(opts, &fail);
		/* If creating the trans failed then it's most likely due to an invalid option. */
		if (trans == NULL) {
			ret = M_GETOPT_ERROR_INVALIDOPT;
		}
	}

	if (ret != M_GETOPT_ERROR_SUCCESS || opts->help) {
		help = cli_opts_gen_help(g, ret, fail, argv[0], opts->help);
		M_printf("%s\n", help);
		M_free(help);

		cli_opts_destroy(opts);
		M_getopt_destroy(g);
		return NULL;
	}

	cli_opts_destroy(opts);
	M_getopt_destroy(g);
	return trans;
}

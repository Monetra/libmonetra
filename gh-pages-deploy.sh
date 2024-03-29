#!/bin/bash
set -e # Exit with nonzero exit code if anything fails

SOURCE_BRANCH="master"
TARGET_BRANCH="main"
PAGES_DIR="_builds/docs/html"
ORIGIN_URL="https://github.com/Monetra/Monetra.github.io.git"
PROJECT_DIR=libmonetra

# GITHUB_TOKEN - set as private variable

echo "Starting deployment"


echo "Target: ${TARGET_BRANCH} branch of ${ORIGIN_URL}"

CURRENT_COMMIT=`git rev-parse HEAD`

ORIGIN_URL_WITH_CREDENTIALS=`echo ${ORIGIN_URL} | sed -e "s|https://.*github\.com/|https://${GITHUB_USER}:${GITHUB_TOKEN}@github.com/|"`

# Clone the existing gh-pages for this repo into gh-pages-deploy/
# Create a new empty branch if gh-pages doesn't exist yet (should only happen on first deply)
echo "Checking out ${TARGET_BRANCH} branch"
git clone -b ${TARGET_BRANCH} --single-branch --depth=1 "${ORIGIN_URL}" gh-pages-deploy

echo "Removing old static content"
rm -rf gh-pages-deploy/${PROJECT_DIR}/**/*

echo "Copying pages content to root"
cp -Rpv ${PAGES_DIR}/* gh-pages-deploy/${PROJECT_DIR}/

echo "Pushing new content to ${ORIGIN_URL}:${TARGET_BRANCH} for CI Deploy ${COMMIT_AUTHOR_EMAIL}"
cd gh-pages-deploy
git config user.name "CI Deploy" || exit 1
git config user.email "${COMMIT_AUTHOR_EMAIL}" || exit 1

# If there are no changes to the compiled out (e.g. this is a README update) then just bail.
if git diff --quiet; then
    echo "No changes to the output on this push; exiting."
    exit 0
fi

echo "Updating working tree..."
git add -A .

echo "Saving working tree..."
git commit -m "Deploy to GitHub Pages: ${CURRENT_COMMIT}"

echo "Pushing to remote as ${GITHUB_USER}..."
git push --quiet "${ORIGIN_URL_WITH_CREDENTIALS}" ${TARGET_BRANCH} > /dev/null 2>&1

cd ..

echo "Deployed successfully."
exit 0

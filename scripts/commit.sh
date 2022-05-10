#!/bin/bash

# this reads all the command line arguments and megerges them into one string
commit_msg="$*"

if [ ${#commit_msg} = 0 ]
then 
    echo Enter commit message:
    read commit_msg
fi

# adds all current files to git, creates a commit with message given as 
# command line arguments or as input then pushes to the current branch if wanted
git add -A

git commit -m "$commit_msg"

# ask user if they want to push the commit 
branch=$(git rev-parse --abbrev-ref HEAD)
echo Do you want to push this commit to $branch ? [y / n]
read push_wish
if [ $push_wish = "y" ]
then
    git push --set-upstream origin $branch
else
    echo not pushing to github
fi

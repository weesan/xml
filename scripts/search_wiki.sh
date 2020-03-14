#!/bin/bash

grep_opt=""

while getopts "i" arg; do
    case "$arg" in
	i) grep_opt="-i $grep_opt"
    esac
done

shift $((OPTIND - 1))

[ $# -eq 2 ] || {
    echo "$0 [-i] <search-term> <wiki-xml>"
    exit -1
}

search_term=$1
wiki_xml=$2
wiki_idx=$(basename $wiki_xml .xml).idx

[ -e $wiki_idx ] || {
    echo "Index $wiki_idx is missing.  Please use index_wiki to create one."
    exit -2
}

while read line; do
    offset=$(echo $line | jq -r .offset)
    length=$(echo $line | jq -r .length)
    echo -n "  "
    dd skip=$offset count=$length bs=1 if=$wiki_xml 2> /dev/null
    echo
done <<EOF
$(grep $grep_opt "$search_term" "$wiki_idx")
EOF

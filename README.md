# XML C++ library

XML C++ library is capable of parsing XML and HTML.  It supports both
DOM (Document Object Model) and SAX (Simple API for XML).

## To compile

```
$ configure
$ make
```

## To pretty format an XML file

```
$ src/prettyxml tests/test.xml
```

## To parse the Wikipedia XML

Since the [Wikipedia](https://www.wikipedia.org/) XML is huge (as of
this writing, ~18GB compressed and ~76GB uncompressed), it's very
challenging to find the desired articles in an efficient matter.  To
speed up, you may use `index_wiki` to index the Wikipedia XML, which
will include the titles and categories.  After that, you may start
searching the Wikipedia documents based on the titles and categories,
which in turn, will tell you the location and length of the documents.

### Download the Wikipedia XML

```
$ wget https://dumps.wikimedia.org/enwiki/20200220/enwiki-20200220-pages-articles-multistream.xml.bz2
```

### Uncompress the Wikipedia XML

```
$ bzip2 -d enwiki-20200220-pages-articles-multistream.xml.bz2
```

### Index the Wikipedia XML

```
$ src/index_wiki < enwiki-20200220-pages-articles-multistream.xml > enwiki-20200220-pages-articles-multistream.idx
```

*Note*: This may take up to a couple of hours depending upon the type of
machine you have.

### Search for the topic

```
$ grep 'Xiaomi Mi 9 Pro' enwiki-20200220-pages-articles-multistream.idx | jq .
{
  "offset": 74594946050,
  "length": 5827,
  "title": "Xiaomi Mi 9 Pro",
  "category": [
    "Android (operating system) devices",
    "Smartphones",
    "Mobile phones introduced in 2019",
    "Xiaomi"
  ]
}
```

### Retrieve the content of the topic

Base on the `offset` and `length` above, you may use the `dd` command
to retrieve the content of the topic.

```
$ dd skip=74594946050 count=5827 bs=1 if=enwiki-20200220-pages-articles-multistream.xml of=xiaomi_mi_9_pro.xml
$ cat xiaomi_mi_9_pro.xml
```

### Using the scripts to retrieve the content of the topic

```
$ scripts/search_wiki.sh 'Mobile phones introduced in 2019' enwiki-20200220-pages-articles-multistream.xml
```

If you prefer a search with case insensitive, you may use `-i` as
follows instead:

```
$ scripts/search_wiki.sh -i 'Mobile phones introduced in 2019' enwiki-20200220-pages-articles-multistream.xml
```

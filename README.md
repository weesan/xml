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
$ wget https://dumps.wikimedia.org/enwiki/20200301/enwiki-20200301-pages-articles-multistream.xml.bz2
```

### Uncompress the Wikipedia XML

```
$ bzip2 -d enwiki-20200301-pages-articles-multistream.xml.bz2
```

### Index the Wikipedia XML

```
$ src/index_wiki < enwiki-20200301-pages-articles-multistream.xml > enwiki-20200301-pages-articles-multistream.idx
```

*Note*: This may take up to a couple of hours depending upon the type of
machine you have.

### Search for the topic

```
$ grep 'Motorola Razr (2020)' enwiki-20200301-pages-articles-multistream.idx | jq .
{
  "offset": 74841758563,
  "length": 12540,
  "title": "Motorola Razr (2020)",
  "category": [
    "Motorola mobile phones",
    "Mobile phones introduced in 2019",
    "Smartphones",
    "Android (operating system) devices",
    "Flexible displays"
  ]
}
```

### Retrieve the content of the topic

Base on the `offset` and `length` above, you may use the `dd` command
to retrieve the content of the topic.

```

$ dd skip=74841758563 count=12540 bs=1 if=enwiki-20200301-pages-articles-multistream.xml of=moto_razr_2020.xml
12540+0 records in
12540+0 records out
12540 bytes (13 kB, 12 KiB) copied, 0.0680268 s, 184 kB/s

$ cat moto_razr_2020.xml
<page>
    <title>Motorola Razr (2020)</title>
    <ns>0</ns>
    <id>62155189</id>
...
  </page>
```

### Using the scripts to retrieve the content of the topic

```
$ scripts/search_wiki.sh 'Motorola Razr (2020)' enwiki-20200301-pages-articles-multistream.xml > moto_razr_2020.xml
$ cat moto_razr_2020.xml
  <page>
    <title>Motorola Razr (2020)</title>
    <ns>0</ns>
    <id>62155189</id>
  ...
  </page>
```

If you prefer a search with case insensitive, you may use `-i` as
follows instead:

```
$ scripts/search_wiki.sh -i 'Motorola Razr (2020)' enwiki-20200301-pages-articles-multistream.xml > moto_razr_2020.xml
$ cat moto_razr_2020.xml
  <page>
    <title>Motorola Razr (2020)</title>
    <ns>0</ns>
    <id>62155189</id>
  ...
  </page>
```

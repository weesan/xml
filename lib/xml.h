#ifndef XML_H
#define XML_H

#include <string>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <unordered_map>
#include <ostream>
#include <stdio.h>
#include <sstream>

using namespace std;

class Element;

typedef bool XML_CB(Element &, void *);

class Hash : public unordered_map<string, Element *> {
};

class Attrs : public vector<pair<string, string> > {
};

class Element : public vector<Element> {
private:
    string        _tag;     // Tag name such as <a>
    string        _content; // The content/text inside the open and close tag.
    Attrs         _attrs;   // Attrs of the tag.  eg. <a href="foo.html">
    Hash          _hash;    // Fast access to the tags under the same parent tag.
    uint64_t      _offset;
    uint32_t      _length;
    Element       *_parent;

private:
    const string startTag(int indent, bool emptyElement = false) const {
        stringstream ss;

        ss << string(indent, ' ').c_str() << '<' << tag();
	if (_attrs.size()) {
	    for (int i = 0; i < _attrs.size(); i++) {
                ss << " " << _attrs[i].first << "=\""<< _attrs[i].second<< "\"";
	    }
	}
	if (emptyElement) {
	    ss << " />";
	} else {
            ss << ">";
	}

        return ss.str();
    }
    const string endTag(int indent = 0) const {
        return string(indent, ' ') + "</" + tag() + ">";
    }

public:
    Element(Element *parent = NULL,
            const char *tag = "",
            uint64_t offset = 0) :
        _parent(parent ? parent : this),
        _tag(tag),
        _offset(offset),
        _length(0) {
    }
    Element *parent(void) const {
        return _parent;
    }
    const string &tag(void) const {
	return _tag;
    }
    void tag(const string &s) {
	_tag = s;
    }
    uint64_t offset(void) const {
        return _offset;
    }
    uint32_t length(void) const {
        return _length;
    }
    void updateLength(uint64_t offset) {
        _length = offset - _offset;
    }
    const string &content(void) const {
	return _content;
    }
    void content(const string &s) {
	_content = s;
    }
    bool empty(void) const {
	return size() == 0 && _content.empty();
    }
    Attrs &attrs(void) {
	return _attrs;
    }
    Hash &hash(void) {
	return _hash;
    }
    virtual ostream &print(ostream &os, int indent = 0) const {
        if (_parent == this) {
            os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
            for (int i = 0; i < size(); i++) {
                (*this)[i].print(os);
            }
            return os;
        }

        if (size()) {
            os << startTag(indent) << endl;
            if (!content().empty()) {
                os << string(indent + 2, ' ') << content() << endl;
            }
            for (int i = 0; i < size(); i++) {
                (*this)[i].print(os, indent + 2);
            }
            os << endTag(indent) << endl;
        } else {
            // Print <tag/> if the tag has empty content; otherwise,
            // print <tag>content</tag> in a single line.
            if (content().empty()) {
                os << startTag(indent, true) << endl;
            } else {
                os << startTag(indent) << content() << endTag() << endl;
            }
        }

        return os;
    }
    Element &lookup(const string &key) {
        // XXX
        return *this;
    }
};

static ostream &operator<<(ostream &o, const Element &e) {
    return e.print(o);
}

class Xml : public Element {
private:
    Element *_current;                // The current tag during parsing.
    XML_CB *_startTagCB, *_endTagCB;  // Various callback functions.
    bool _debug;

private:
    void traverse(bool (*cb)(Element &, void *),
                  Element &e, void *ctx) {
	assert(cb);
	// If the callback returns false, terminate early.
	if (!cb(e, ctx)) {
	    return;
	}
	for (int i = 0; i < e.size(); i++) {
	    traverse(cb, e[i], ctx);
	}
    }

public:
    Xml(XML_CB *startTagCB = NULL, XML_CB *endTagCB = NULL, bool debug = false):
        _current(this),
        _startTagCB(startTagCB),
        _endTagCB(endTagCB),
        _debug(debug) {
    }
    bool debug(void) const {
        return _debug;
    }
    void parse(void) {
	extern int parseXml(Xml &xml);
	parseXml(*this);
    }
    void reset(void) {
        clear();
        _current = this;
    }
    void updateLength(uint64_t offset) {
        _current->updateLength(offset);
    }
    void up(void) {
        _current = _current->parent();
    }
    Element &addChild(const char *tag, uint64_t offset) {
        _current->push_back(Element(_current, tag, offset));
	// Set current to be newly added element.
        Element &e = _current->back();
        _current = &e;
        return e;
    }
    void addContent(const char *content) {
        _current->content(content);
    }
    void appendContent(const char *content) {
        _current->content(_current->content() + content);
    }
    void traverse(XML_CB *cb, void *ctx = NULL) {
	if (!cb) {
	    throw runtime_error("Failed to provide a callback function!");
	}
	// Skip the root, and start from children.
	for (int i = 0; i < size(); i++) {
	    traverse(cb, (*this)[i], ctx);
	}
    }
    void startTag(void) {
        if (_debug) {
            fprintf(stderr, "Start tag callback\n");
        }
        if (_startTagCB) {
            _startTagCB(*_current, NULL);
        }
    }
    void endTag(void) {
        if (_debug) {
            fprintf(stderr, "End tag callback\n");
        }
        if (_endTagCB) {
            _endTagCB(*_current, NULL);
        }
    }
};

#endif // XML_H

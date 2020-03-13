#include "xml.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>
#include <string>

using namespace std;

static
bool page_cb(Element &e, void *ctx)
{
    if (e.tag() == "page") {
        //printf("Offset: %lu\n", e.offset());
        //printf("length: %lu\n", e.length());

        stringstream ss;
        ss << e;
        string s = ss.str();
        //cout << s;

        string title;
        vector<string> categories;

        smatch sm;

        // Only one title.
        if (regex_search(s, sm, regex("<title>(.*)</title>"))) {
            //cout << "[" << sm[1] << "]" << endl;
            title = sm[1];
        }

        // Could be multiple categories or none at all.
        while (regex_search(s, sm, regex("\\[\\[Category:(.*)\\]\\]"))) {
            //cout << "[" << sm[1] << "]" << endl;
            categories.push_back(sm[1]);
            s = sm.suffix().str();
        }

        cout << "{"
             << "\"offset\":" << e.offset() << ","
             << "\"length\":" << e.length() << ","
             << "\"title\":"  << "\"" << title << "\"";
        if (categories.size()) {
            cout << ",\"category\": [";
            for (int i = 0; i < categories.size(); i++) {
                if (i != 0) {
                    cout << ",";
                }
                cout << "\"" << categories[i] << "\"";
            }
            cout << "]";
        }
        cout << "}" << endl;

        // Delete <page>...</page> to conserve memory.
        e.parent()->pop_back();
    }

    return true;
}

int main (void)
{
    Xml xml(NULL, page_cb);
    xml.parse();
    //cout << xml;
    //xml.traverse(page_cb);
    return 0;
}

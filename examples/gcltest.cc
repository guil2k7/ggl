// Copyright 2025 Maicol Castro <maicolcastro.abc@gmail.com>.
// Distributed under the MIT License.
// See LICENSE.txt in the root directory of this project
// or at https://opensource.org/license/mit.

#include <iostream>
#include <fstream>
#include <gcl/exception.hh>
#include <gcl/parser.hh>

class GclValuePrinter final : public gcl::IValueVisitor {
public:
    GclValuePrinter(size_t tabSize = 4) : tabSize{tabSize}, m_tabs{0} {}

    void visit() override {
        std::cout << "undefined" << std::flush;
    }

    void visit(std::nullptr_t) override {
        std::cout << "null" << std::flush;
    }

    void visit(bool val) override {
        if (val)
            std::cout << "true" << std::flush;
        else
            std::cout << "false" << std::flush;
    }

    void visit(intptr_t val) override {
        std::cout << val << std::flush;
    }

    void visit(float val) override {
        std::cout << val << std::flush;
    }

    void visit(gcl::String const& string) override {
        std::cout << '"' << string << '"' << std::flush;
    }

    void visit(gcl::Array const& array) override {
        if (array.size() == 0) {
            std::cout << "[]" << std::flush;
            return;
        }

        std::cout << '[' << std::endl;
        ++m_tabs;

        auto it = array.begin();

        for (;;) {
            printTabs();
            it->accept(*this);

            ++it;

            if (it != array.end())
                std::cout << ',' << std::endl;
            else
                break;
        }

        --m_tabs;

        std::cout << std::endl;
        printTabs();
        std::cout << ']' << std::flush;
    }

    void visit(gcl::Dict const& dict) override {
        if (dict.size() == 0) {
            std::cout << "{}" << std::flush;
            return;
        }

        std::cout << '{' << std::endl;
        ++m_tabs;

        auto it = dict.begin();

        for (;;) {
            printTabs();

            std::cout << it->first << ": " << std::flush;
            it->second.accept(*this);

            ++it;

            if (it != dict.end())
                std::cout << ',' << std::endl;
            else
                break;
        }

        --m_tabs;

        std::cout << std::endl;
        printTabs();
        std::cout << '}' << std::flush;
    }

    size_t tabSize;

private:
    void printTabs() {
        for (size_t i = 0; i < m_tabs * tabSize; ++i)
            std::cout.put(' ');
    }

    size_t m_tabs;
};

static void printGcl(gcl::Value const& value, size_t tabSize = 4) {
    GclValuePrinter printer(tabSize);
    value.accept(printer);

    std::cout << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "use: /gcltest [file]" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);

    if (!file.is_open()) {
        std::cout << "error: could not open file \"" << argv[1] << '"' << std::endl;
        return 1;
    }

    file.seekg(0, std::ios::end);
    size_t length = file.tellg();
    file.seekg(0, std::ios::beg);

    char* text = new char[length];

    file.read(text, length);
    file.close();

    gcl::Value result;

    try {
        gcl::parse(result, text, length);
    }
    catch (gcl::GclException const& exception) {
        std::cout << "[GCL Error]: " << exception.info << std::endl;
        return 1;
    }

    std::cout << "----------------------------------------------------------------" << std::endl;
    printGcl(result);
    std::cout << "----------------------------------------------------------------" << std::endl;

    delete[] text;

    return 0;
}

// lsp_server_test.cpp
// Test suite for Alterion LSP server
#include "lsp_server.h"
#include <iostream>

int main() {
    LSPServer lsp;
    lsp.start(8080);
    lsp.handleRequest("textDocument/didOpen");
    std::cout << "LSP Response: " << lsp.getResponse() << std::endl;
    return 0;
}

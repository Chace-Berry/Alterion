// lsp_server.cpp
// Alterion Language - Phase 6: Language Server Protocol (LSP) Integration
#include "lsp_server.h"
#include <iostream>

LSPServer::LSPServer() {}

void LSPServer::start(int port) {
    std::cout << "LSP server started on port " << port << std::endl;
}

void LSPServer::handleRequest(const std::string& request) {
    // Example: handle request and generate diagnostics
    diagnostics.push_back("Handled request: " + request);
    lastResponse = "Response to: " + request;
}

std::string LSPServer::getResponse() const {
    return lastResponse;
}

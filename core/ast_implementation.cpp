#include "ast.h"
#include <sstream>

std::string astToJson(const std::shared_ptr<ASTNode>& node, int indent) {
    std::ostringstream oss;
    std::string ind(indent, ' ');
    oss << ind << "{\n";
    oss << ind << "  \"type\": \"" << static_cast<int>(node->type) << "\",\n";
    oss << ind << "  \"name\": \"" << node->name << "\",\n";
    if (node->value)
        oss << ind << "  \"value\": \"" << *node->value << "\",\n";
    oss << ind << "  \"line\": " << node->line << ",\n";
    oss << ind << "  \"column\": " << node->column << ",\n";
    if (node->error)
        oss << ind << "  \"error\": \"" << *node->error << "\",\n";
    oss << ind << "  \"children\": [\n";
    for (size_t i = 0; i < node->children.size(); ++i) {
        oss << astToJson(node->children[i], indent + 4);
        if (i + 1 < node->children.size()) oss << ",\n";
    }
    oss << "\n" << ind << "  ]\n";
    oss << ind << "}";
    return oss.str();
}

#include "aurora/AST.h"

namespace aurora {

// Generate implicit constructor if no explicit constructor exists
void ClassDecl::generateImplicitConstructor() {
    // Check if a constructor already exists
    for (const auto& method : methods) {
        if (method.isConstructor) {
            return;  // Explicit constructor exists, don't generate implicit one
        }
    }
    
    // Generate implicit constructor with all fields as parameters
    std::vector<Parameter> params;
    std::vector<std::unique_ptr<Stmt>> body;
    
    auto& registry = TypeRegistry::instance();
    auto classType = registry.getClassType(name);
    
    for (const auto& field : fields) {
        // Add parameter for each field
        params.push_back(Parameter(field.name, field.type));
        
        // Generate assignment: this.fieldName = paramName
        auto thisExpr = std::make_unique<ThisExpr>(classType);
        auto memberAccess = std::make_unique<MemberAccessExpr>(
            std::move(thisExpr), field.name, field.type);
        auto paramExpr = std::make_unique<VariableExpr>(field.name, field.type);
        auto assignment = std::make_unique<AssignStmt>(
            std::move(memberAccess), std::move(paramExpr));
        body.push_back(std::move(assignment));
    }
    
    // Create implicit constructor method
    MethodDecl implicitCtor(
        "constructor",
        std::move(params),
        registry.getVoidType(),
        std::move(body),
        true,   // isPublic
        false,  // isStatic
        true    // isConstructor
    );
    
    methods.push_back(std::move(implicitCtor));
}

} // namespace aurora


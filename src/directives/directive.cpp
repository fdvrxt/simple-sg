#include "directive.hpp"
#include "index.hpp"

std::unique_ptr<Directive> getDirective(const std::string& name)
{
    static const std::unordered_map<std::string,
        std::function<std::unique_ptr<Directive>()>> creators{
        {"index", [] { return std::make_unique<Index>(); }},
    };

    if (auto it = creators.find(name); it != creators.end())
        return it->second();

    return nullptr; 
}
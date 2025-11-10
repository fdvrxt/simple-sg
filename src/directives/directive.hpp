#ifndef DIRECTIVE_H_
#define DIRECTIVE_H_

#include <nlohmann/json.hpp>
#include <inja.hpp>
#include "../data/config.hpp"
#include "../utils/debug.hpp"
#include <inja.hpp>

class Directive {
public:
	virtual void init(Config& config, const nlohmann::json directive) = 0;
	virtual void render() = 0;
};

std::unique_ptr<Directive> getDirective(const std::string& name);

#endif

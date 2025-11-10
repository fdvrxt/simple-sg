#ifndef INDEX_HPP_
#define INDEX_HPP_

#include <nlohmann/json.hpp>
#include <inja.hpp>
#include "directive.hpp"
#include "../utils/debug.hpp"
#include "../data/config.hpp"

class Index : public Directive {
private:
	int count = 0;

public:
	Index();
	virtual void init(Config& config, const nlohmann::json directive);
	virtual void render();
};

#endif

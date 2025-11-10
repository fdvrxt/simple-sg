#ifndef TAGS_HPP_
#define TAGS_HPP_

#include <nlohmann/json.hpp>
#include <inja.hpp>
#include "directive.hpp"
#include "../utils/debug.hpp"
#include "../data/config.hpp"
#include "../utils/utils.hpp"

class Tags : public Directive {
public:
    Tags() = default;
    virtual void init(Config& config, const nlohmann::json directive) override;
    virtual void render() override;
};

#endif
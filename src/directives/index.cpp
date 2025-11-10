#include "index.hpp"

Index::Index()
{

}

void Index::init(Config& config, const nlohmann::json directive)
{
	int number_of_pages = config.getData().getJson()["pages"].size();
	int posts_per_page = directive["count"];
	const inja::Template temp = config.getTemplate(directive["path"]);

}

void Index::render() 
{

}

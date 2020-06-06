#include "cached_page.h"

bool cached_page::isExpired() 
{
	// time(0) returns current time.
	return time(0) - time_start > max_age;
}

Response cached_page::get_cache()
{
	return response;
}

void cached_page::update_cache(Response res)
{
	response = res;
}

void cached_page::update_age(int age)
{
	max_age = age;
}
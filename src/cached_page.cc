#include "cached_page.h"

bool cached_page::isExpired(int max)
{
	// time(0) returns current time.
	return time(0) - time_start > max;
}

bool cached_page::isFresh(int min_fresh)
{
	// the cache is at least this fresh
	return time(0) > time_start + min_fresh;
}

/*
	return true if the cache exceeds its expiration time
	no later than the given value.
*/
bool cached_page::couldStale(int max_stale)
{
	int stale_time = time(0) - time_start - max_age;
	return stale_time < max_stale;
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

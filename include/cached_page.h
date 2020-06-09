#include "response.h"
#include <string>
#include <time.h>

// default age set by the server
const int one_min = 60;

class cached_page {
	public:
		cached_page(Response res, int age = one_min)
		: response(res), time_start(time(0)), max_age(age) {}

		bool isExpired(int max = one_min);
		bool isFresh(int min_fresh);
		bool couldStale(int max_stale);
		Response get_cache();
		void update_cache(Response res);
		void update_age(int age);

	private:
		Response response;
		time_t time_start;
		int max_age;
};

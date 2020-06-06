#include "response.h"
#include <string>
#include <time.h>

const int ten_mins = 60 * 10;

class cached_page {
	public:
		cached_page(Response res, int age = ten_mins) 
		: response(res), time_start(time(0)), max_age(age) {}

		bool isExpired();
		Response get_cache();
		void update_cache(Response res);
		void update_age(int age);

	private:
		Response response;
		time_t time_start;
		int max_age;
};
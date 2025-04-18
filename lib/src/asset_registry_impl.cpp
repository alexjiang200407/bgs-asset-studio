#include "asset_registry_impl.h"

using namespace std;

void asset_registry_impl::insert(asset_ptr asset) { this->assets.push(asset); }

void asset_registry_impl::process_all(size_t num_threads)
{
	std::vector<std::thread> threads;

	auto worker_func = [&]() {
		asset_ptr asset;
		while (true)
		{
			if (this->assets.try_pop(asset))
			{
				asset->process();
			}
			else
			{
				spdlog::info("Finished");
				break;
			}
		}
	};

	for (size_t i = 0; i < num_threads; i++)
	{
		threads.emplace_back(worker_func);
	}

	for (auto& t : threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
}

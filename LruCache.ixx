export module com.ntmonkeys.library.lrucache;

import com.ntmonkeys.library.unique;
import <concepts>;
import <list>;
import <unordered_map>;
import <stdexcept>;
import <memory>;

namespace NT::Lib
{
	export template <std::copy_constructible $Key, std::move_constructible $Value>
	class LruCache : public Unique
	{
	public:
		LruCache(const size_t capacity) noexcept;

		[[nodiscard]]
		bool isCached(const $Key &key) const noexcept;

		[[nodiscard]]
		const $Value &get(const $Key &key);
		void set(const $Key &key, $Value &&value) noexcept;

		void clear() noexcept;

		[[nodiscard]]
		const $Value &operator[](const $Key &key);

	private:
		using __KeyRefOrder = typename std::list<$Key>::iterator;

		struct __CacheInfo
		{
		public:
			__KeyRefOrder keyRefOrder;
			$Value value;
		};

		const size_t __capacity;
		std::list<$Key> __keyReferences;
		std::unordered_map<$Key, std::unique_ptr<__CacheInfo>> __cacheInfoMap;
	};

	template <std::copy_constructible $Key, std::move_constructible $Value>
	LruCache<$Key, $Value>::LruCache(const size_t capacity) noexcept :
		__capacity{ capacity }
	{}

	template <std::copy_constructible $Key, std::move_constructible $Value>
	bool LruCache<$Key, $Value>::isCached(const $Key &key) const noexcept
	{
		return __cacheInfoMap.contains(key);
	}

	template <std::copy_constructible $Key, std::move_constructible $Value>
	const $Value &LruCache<$Key, $Value>::get(const $Key &key)
	{
		const auto foundIt{ __cacheInfoMap.find(key) };
		if (foundIt == __cacheInfoMap.end())
			throw std::runtime_error{ "There is no value corresponding to the key." };

		__CacheInfo &cacheInfo{ *(foundIt->second) };
		__keyReferences.erase(cacheInfo.keyRefOrder);
		__keyReferences.emplace_front(key);

		cacheInfo.keyRefOrder = __keyReferences.begin();
		return cacheInfo.value;
	}

	template <std::copy_constructible $Key, std::move_constructible $Value>
	void LruCache<$Key, $Value>::set(const $Key &key, $Value &&value) noexcept
	{
		const auto foundIt{ __cacheInfoMap.find(key) };
		if (foundIt != __cacheInfoMap.end())
		{
			__CacheInfo &cacheInfo{ *(foundIt->second) };
			__keyReferences.erase(cacheInfo.keyRefOrder);
			__keyReferences.emplace_front(key);

			cacheInfo.keyRefOrder = __keyReferences.begin();
			cacheInfo.value = std::move(value);
		}
		else
		{
			if (__capacity == __keyReferences.size())
			{
				__cacheInfoMap.erase(__keyReferences.back());
				__keyReferences.pop_back();
			}

			__keyReferences.emplace_front(key);
			__cacheInfoMap[key] = std::make_unique<__CacheInfo>(__keyReferences.begin(), std::move(value));
		}
	}

	template <std::copy_constructible $Key, std::move_constructible $Value>
	void LruCache<$Key, $Value>::clear() noexcept
	{
		__keyReferences.clear();
		__cacheInfoMap.clear();
	}

	template <std::copy_constructible $Key, std::move_constructible $Value>
	const $Value &LruCache<$Key, $Value>::operator[](const $Key &key)
	{
		return get(key);
	}
}
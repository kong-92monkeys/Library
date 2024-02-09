export module com.ntmonkeys.library.lrucache;

import com.ntmonkeys.library.unique;
import <concepts>;
import <list>;
import <unordered_map>;
import <stdexcept>;
import <memory>;

namespace NT::Lib
{
	/// <summary>
	/// This is a data structure with a fixed size, capable of storing data only up to that size.
	/// Data is stored as key-value pairs, and if you know the key used to store a value,
	/// you can retrieve the stored value later.
	/// If you try to store a value exceeding the fixed size,
	/// the key that has not been used for the longest time is automatically deleted.
	/// </summary>
	/// <typeparam name="$Key">The key type. It must be a type that can be copy-constructible.</typeparam>
	/// <typeparam name="$Value">The value type. It must be a type that can be move-constructible.</typeparam>
	export template <std::copy_constructible $Key, std::move_constructible $Value>
	class LruCache : public Unique
	{
	public:
		/// <summary>
		/// This is the constructor. It takes the cache size as an argument.
		/// </summary>
		/// <param name="capacity">Cache size</param>
		LruCache(const size_t capacity) noexcept;

		/// <summary>
		/// Returns whether there is a value mapped to the specified key.
		/// It returns false if the key-value pair has never been stored,
		/// or if the value has been deleted due to the LRU algorithm.
		/// </summary>
		/// <param name="key">The key to investigate</param>
		/// <returns>Whether there is a mapped value.</returns>
		[[nodiscard]]
		bool isCached(const $Key &key) const noexcept;

		/// <summary>
		/// Returns the value mapped to the specified key.
		/// If there is no mapped value, a std::runtime_error is thrown.
		/// You should call the isCached(key) function before calling this function to prevent the exception from occurring.
		/// Calling this function updates the LRU order to the most recent.
		/// </summary>
		/// <param name="key">The key to use for finding the value.</param>
		/// <returns>The value corresponding to the input key.</returns>
		[[nodiscard]]
		const $Value &get(const $Key &key);

		/// <summary>
		/// Maps a new value to the specified key.
		/// Calling this function updates the LRU order to the most recent.
		/// </summary>
		/// <param name="key">The key to use for mapping the value.</param>
		/// <param name="value">The value</param>
		void set(const $Key &key, $Value &&value) noexcept;
		
		/// <summary>
		/// Initializes all cached data.
		/// </summary>
		void clear() noexcept;

		/// <summary>
		/// Returns the value mapped to the specified key.
		/// Performs the same function as the get() function.
		/// </summary>
		/// <param name="key">The key to use for finding the value.</param>
		/// <returns>The value corresponding to the input key.</returns>
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
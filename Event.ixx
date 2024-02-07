export module com.ntmonkeys.library.event;

import com.ntmonkeys.library.unique;
import <functional>;
import <memory>;
import <unordered_map>;
import <stdexcept>;

export namespace NT::Lib
{
	template <typename ...$Args>
	class EventListener;

	template <typename ...$Args>
	using EventCallback = std::function<void(const $Args &...)>;

	/// <summary>
	/// This is a module for detecting the occurrence of events.
	/// You can register for specific events or remove them.
	/// When an event occurs, the callback function passed to the constructor is called.
	/// </summary>
	/// <typeparam name="$Args">The list of argument types passed when an event occurs.</typeparam>
	template <typename ...$Args>
	class EventListener : public Unique
	{
	public:
		/// <summary>
		/// This is the constructor of the event listener object.
		/// It takes a function as an argument, which will be called when an event occurs.
		/// </summary>
		/// <param name="callback">The function that will be called when an event occurs.</param>
		EventListener(EventCallback<$Args...> callback);

		/// <summary>
		/// Calls the function registered as a callback.
		/// </summary>
		/// <param name="args">The argument value to pass to the callback function as a parameter.</param>
		void send(const $Args &...args);

		/// <summary>
		/// Creates and returns an event listener object of type shared_ptr.
		/// </summary>
		/// <param name="callback">The function that will be called when an event occurs.</param>
		/// <returns>An event listener object of type shared_ptr.</returns>
		[[nodiscard]]
		static std::shared_ptr<EventListener<$Args...>> make(EventCallback<$Args...> callback);

		/// <summary>
		/// Creates and returns an event listener object of type shared_ptr. It uses the std::bind() function to create the callback function.
		/// </summary>
		/// <typeparam name="$Params">The list of type parameters to be passed to the std::bind() function.</typeparam>
		/// <param name="params">The list of arguments to be passed to the std::bind() function.</param>
		/// <returns>An event listener object of type shared_ptr.</returns>
		template <typename ...$Params>
		[[nodiscard]]
		static std::shared_ptr<EventListener<$Args...>> bind($Params &&...params) noexcept;

	private:
		EventCallback<$Args...> __callbackFunc;
	};

	/// <summary>
	/// This is a public view interface for the event object.
	/// Only event listener registration/removal operations are possible.
	/// </summary>
	/// <typeparam name="$Args">The list of argument types passed when an event occurs.</typeparam>
	template <typename ...$Args>
	class EventView : public Unique
	{
	public:
		/// <summary>
		/// Registers an event listener.
		/// If the same listener object is registered more than once, the reference count is increased.
		/// </summary>
		/// <param name="pListener">The event listener object you want to register.</param>
		virtual void addListener(const std::shared_ptr<EventListener<$Args...>> &pListener) = 0;

		/// <summary>
		/// Removes an event listener.
		/// If the reference count of the listener instance is greater than 2, only the reference count is reduced.
		/// </summary>
		/// <param name="pListener">The event listener object you want to remove.</param>
		virtual void removeListener(const std::shared_ptr<EventListener<$Args...>> &pListener) = 0;

		/// <summary>
		/// Performs the same function as the addListener() function.
		/// </summary>
		/// <param name="pListener">The event listener object you want to register.</param>
		EventView &operator+=(const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept;

		/// <summary>
		/// Performs the same function as the removeListener() function.
		/// </summary>
		/// <param name="pListener">The event listener object you want to remove.</param>
		EventView &operator-=(const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept;
	};

	/// <summary>
	/// This is a module that can trigger events. It implements the EventView class.
	/// The event owner must provide the event instance as an EventView type to prevent external sources from triggering the event.
	/// </summary>
	/// <typeparam name="$Args">The list of argument types passed when an event occurs.</typeparam>
	template <typename ...$Args>
	class Event : public EventView<$Args...>
	{
	public:
		/// <summary>
		/// Registers an event listener.
		/// If the same listener object is registered more than once, the reference count is increased.
		/// </summary>
		/// <param name="pListener">The event listener object you want to register.</param>
		virtual void addListener(const std::shared_ptr<EventListener<$Args...>> &pListener) override;

		/// <summary>
		/// Removes an event listener.
		/// If the reference count of the listener instance is greater than 2, only the reference count is reduced.
		/// </summary>
		/// <param name="pListener">The event listener object you want to remove.</param>
		virtual void removeListener(const std::shared_ptr<EventListener<$Args...>> &pListener) override;

		/// <summary>
		/// Triggers the event.
		/// </summary>
		/// <param name="args">The event argument list.</param>
		void invoke(const $Args &...args);

	private:
		std::unordered_map<
			EventListener<$Args...> *,
			std::pair<size_t, std::weak_ptr<EventListener<$Args...>>>> __listenerRefMap;
	};

	template <typename ...$Args>
	EventListener<$Args...>::EventListener(EventCallback<$Args...> callback) :
		__callbackFunc{ std::move(callback) }
	{
		if (!__callbackFunc)
			throw std::runtime_error{ "The callback function cannot be null." };
	}

	template <typename ...$Args>
	void EventListener<$Args...>::send(const $Args &...args)
	{
		__callbackFunc(args...);
	}

	template <typename ...$Args>
	std::shared_ptr<EventListener<$Args...>> EventListener<$Args...>::make(EventCallback<$Args...> callback)
	{
		return std::make_shared<EventListener<$Args...>>(std::move(callback));
	}

	template <typename ...$Args>
	template <typename ...$Params>
	std::shared_ptr<EventListener<$Args...>> EventListener<$Args...>::bind($Params &&...params) noexcept
	{
		return make(std::bind(std::forward<$Params>(params)...));
	}

	template <typename ...$Args>
	EventView<$Args...> &EventView<$Args...>::operator+=(const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept
	{
		addListener(pListener);
		return *this;
	}

	template <typename ...$Args>
	EventView<$Args...> &EventView<$Args...>::operator-=(const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept
	{
		removeListener(pListener);
		return *this;
	}

	template <typename ...$Args>
	void Event<$Args...>::addListener(const std::shared_ptr<EventListener<$Args...>> &pListener)
	{
		auto &[refCount, wpListener] { __listenerRefMap[pListener.get()] };
		++refCount;
		wpListener = pListener;
	}

	template <typename ...$Args>
	void Event<$Args...>::removeListener(const std::shared_ptr<EventListener<$Args...>> &pListener)
	{
		auto &[refCount, wpListener] { __listenerRefMap[pListener.get()] };
		--refCount;
	}

	template <typename ...$Args>
	void Event<$Args...>::invoke(const $Args &...args)
	{
		for (auto iter{ __listenerRefMap.begin() }; iter != __listenerRefMap.end(); )
		{
			const auto &[pListener, refHolder] { *iter };
			const auto &[refCount, wpListener] { refHolder };

			if (!refCount || wpListener.expired())
				iter = __listenerRefMap.erase(iter);
			else
			{
				pListener->send(args...);
				iter++;
			}
		}
	}
}
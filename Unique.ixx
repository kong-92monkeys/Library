export module com.ntmonkeys.library.unique;

export namespace NT::Lib
{
	/// <summary>
	/// This is a class used when you want to declare that instances of the class you are implementing are non-copyable and non-movable.
	/// If you inherit this class, that functionality is automatically applied.
	/// </summary>
	class Unique
	{
	public:
		Unique() = default;
		Unique(const Unique &) = delete;
		Unique(Unique &&) = delete;

		virtual ~Unique() noexcept = default;

		Unique &operator=(const Unique &) = delete;
		Unique &operator=(Unique &&) = delete;
	};
}
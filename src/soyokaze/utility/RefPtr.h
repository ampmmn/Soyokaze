#pragma once

template <typename T>
class RefPtr
{
public:
	RefPtr() : mPtr(nullptr){}

	RefPtr(T* ptr, bool isAddRefCount = false) : mPtr(ptr) {
		if (ptr && isAddRefCount) {
			ptr->AddRef();
		}
	}

	RefPtr(RefPtr<T>&& rhs) noexcept : mPtr(rhs.mPtr) {
		rhs.mPtr = nullptr;
	}

	RefPtr(RefPtr<T>& rhs) : mPtr(rhs.mPtr) {
		if (rhs.mPtr) {
			rhs->AddRef();
		}
	}

	~RefPtr() {
		if (mPtr) {
			mPtr->Release();
		}
	}

	operator T*() { return mPtr; }
	operator const T*() const { return mPtr; }
	T* operator ->() const noexcept { return mPtr; }
	T** operator&() { return &mPtr; }

	T* get() { return mPtr; }
	const T* get() const { return mPtr; }

	T* release() {
		T* p = mPtr;
		mPtr = nullptr;
		return p;
	}

	void reset(T* ptr = nullptr) {
		if (mPtr) {
			mPtr->Release();
		}
		mPtr = ptr;
	}

	void swap(RefPtr<T>& rhs) {
		T* tmp = mPtr;
		mPtr = rhs.mPtr;
		rhs.mPtr = tmp;
	}

private:
	T* mPtr = nullptr;

};

template <class T, class... _Types>
RefPtr<T> make_refptr(_Types&&... _Args)
{
    return RefPtr<T>(new T(std::forward<_Types>(_Args)...));
}

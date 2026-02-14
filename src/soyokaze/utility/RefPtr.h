#pragma once

#include <algorithm> // for std::swap

template <typename T>
class RefPtr
{
public:
	RefPtr() : mPtr(nullptr){}

	RefPtr(T* ptr, bool isAddRefCount = false) : mPtr(ptr) {
		if (ptr && isAddRefCount) {
#ifndef _DEBUG
			ptr->AddRef();
#else
			auto n = mPtr->AddRef();
			if (mIsDebugMode) { spdlog::debug("AddRef {0}(->{1})", (void*)mPtr, n); }
#endif
		}
	}

	RefPtr(RefPtr<T>&& rhs) noexcept : mPtr(rhs.mPtr) {
		rhs.mPtr = nullptr;
#ifdef _DEBUG
		mIsDebugMode = rhs.mIsDebugMode;
#endif
	}

	RefPtr(const RefPtr<T>& rhs) : mPtr(rhs.mPtr) {
		if (mPtr) {
#ifndef _DEBUG
			mPtr->AddRef();
#else
			auto n = mPtr->AddRef();
			if (mIsDebugMode) { spdlog::debug("AddRef {0}(->{1})", (void*)mPtr, n); }
			mIsDebugMode = rhs.mIsDebugMode;
#endif
		}
	}

	~RefPtr() {
		if (mPtr) {
#ifndef _DEBUG
			mPtr->Release();
#else
			auto n = mPtr->Release();
			if (mIsDebugMode) { spdlog::debug("Release {0}(->{1})", (void*)mPtr, n); }
#endif
			mPtr = nullptr;
		}
	}

	operator T*() { return mPtr; }
	operator const T*() const { return mPtr; }
	T* operator ->() const noexcept { return mPtr; }
	T** operator&() { return &mPtr; }

	RefPtr<T>& operator = (const RefPtr<T>& rhs) {
		if (rhs.mPtr != mPtr) {
#ifdef _DEBUG
			mIsDebugMode = rhs.mIsDebugMode;
#endif
			if (mPtr != nullptr) {
#ifndef _DEBUG
				mPtr->Release();
#else
				auto n = mPtr->Release();
				if (mIsDebugMode) { spdlog::debug("Release {0}(->{1})", (void*)mPtr, n); }
#endif
			}
			mPtr = rhs.mPtr;
			if (mPtr != nullptr) {
#ifndef _DEBUG
				mPtr->AddRef();
#else
				auto n = mPtr->AddRef();
				if (mIsDebugMode) { spdlog::debug("AddRef {0}(->{1})", (void*)mPtr, n); }
#endif
			}
		}
		return *this;
	}

	T* get() { return mPtr; }
	const T* get() const { return mPtr; }

	T* release() {
		T* p = mPtr;
		mPtr = nullptr;
		return p;
	}

	void reset(T* ptr = nullptr) {
		if (mPtr) {
#ifndef _DEBUG
			mPtr->Release();
#else
			auto n = mPtr->Release();
			if (mIsDebugMode) { spdlog::debug("Release {0}(->{1})", (void*)mPtr, n); }
#endif
		}
		mPtr = ptr;
	}

	void swap(RefPtr<T>& rhs) {
		std::swap(mPtr, rhs.mPtr);
#ifdef _DEBUG
		std::swap(mIsDebugMode, rhs.mIsDebugMode);
#endif
	}

private:
	T* mPtr{nullptr};

public:
#ifndef _DEBUG
	void setDebugMode(bool) {}
#else
	void setDebugMode(bool isDbgMode) {
		mIsDebugMode = isDbgMode;
	}
	bool mIsDebugMode{false};
#endif

};

template <class T, class... _Types>
RefPtr<T> make_refptr(_Types&&... _Args)
{
    return RefPtr<T>(new T(std::forward<_Types>(_Args)...));
}

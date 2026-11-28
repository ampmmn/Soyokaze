#pragma once

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace workspaceplugin {

using EntryId = uint32_t;
using StringId = uint32_t;
using RootId = uint32_t;

constexpr EntryId InvalidEntryId = (std::numeric_limits<EntryId>::max)();
constexpr StringId InvalidStringId = (std::numeric_limits<StringId>::max)();
constexpr RootId InvalidRootId = (std::numeric_limits<RootId>::max)();

/**
	文字列を重複なく保持し、IDを介して参照する
	*/
class StringPool
{
private:

	struct Record {
		std::wstring value;
		std::string utf8;
	};

	struct StringViewHash {
		using is_transparent = void;

		size_t operator()(std::wstring_view value) const noexcept
		{
			return std::hash<std::wstring_view>{}(value);
		}
	};

	struct StringViewEqual {
		using is_transparent = void;

		bool operator()(std::wstring_view lhs, std::wstring_view rhs) const noexcept
		{
			return lhs == rhs;
		}
	};

	std::vector<std::unique_ptr<Record>> mRecords;
	std::unordered_map<std::wstring_view, StringId, StringViewHash, StringViewEqual> mIds;

	/**
		UTF-16文字列をUTF-8文字列へ変換する
		@param[in] value 変換する文字列
		@return UTF-8文字列。変換できない場合は空文字列
		*/
	static std::string ToUtf8(std::wstring_view value)
	{
		if (value.empty()) return {};
		if (value.size() > static_cast<size_t>((std::numeric_limits<int>::max)())) return {};
		const int sourceLength = static_cast<int>(value.size());
		const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
				sourceLength, nullptr, 0, nullptr, nullptr);
		if (length <= 0) return {};
		std::string result(static_cast<size_t>(length), '\0');
		if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), sourceLength,
					result.data(), length, nullptr, nullptr) != length) return {};
		return result;
	}

public:
	/**
		文字列を登録し、対応するIDを返す
		@param[in] value 登録する文字列
		@return 文字列ID
		*/
	StringId Intern(std::wstring_view value)
	{
		const auto found = mIds.find(value);
		if (found != mIds.end()) return found->second;
		if (mRecords.size() >= static_cast<size_t>(InvalidStringId)) {
			throw std::length_error("StringPool capacity exceeded.");
		}

		auto record = std::make_unique<Record>();
		record->value.assign(value);
		record->utf8 = ToUtf8(value);
		const StringId id = static_cast<StringId>(mRecords.size());
		const std::wstring_view key(record->value);
		mRecords.push_back(std::move(record));
		try {
			const auto inserted = mIds.emplace(key, id);
			if (!inserted.second) {
				mRecords.pop_back();
				return inserted.first->second;
			}
		}
		catch (...) {
			mRecords.pop_back();
			throw;
		}
		return id;
	}

	/**
		IDに対応する文字列を取得する
		@param[in] id 文字列ID
		@return 文字列ビュー。無効なIDの場合は空文字列
		*/
	std::wstring_view Get(StringId id) const
	{
		if (id >= mRecords.size()) return {};
		return mRecords[id]->value;
	}

	/**
		IDに対応するUTF-8文字列を取得する
		@param[in] id 文字列ID
		@return UTF-8文字列ビュー。無効なIDの場合は空文字列
		*/
	std::string_view GetUtf8(StringId id) const
	{
		if (id >= mRecords.size()) return {};
		return mRecords[id]->utf8;
	}

	/**
		登録されている文字列数を取得する
		@return 文字列数
		*/
	size_t Size() const
	{
		return mRecords.size();
	}
};

// 検索対象項目。パスは親IDと文字列IDから必要時に復元する。
struct Entry
{
	// 親ディレクトリの項目ID。ルート直下の場合は無効値。
	EntryId parentId{InvalidEntryId};
	// 文字列プールに登録された項目名のID。
	StringId nameId{InvalidStringId};
	// 対象項目がディレクトリの場合はtrue。
	bool isDirectory{false};
	// 項目が属するルート情報のID。
	RootId rootId{InvalidRootId};
};

// ルートディレクトリと所属ワークスペースの情報。
struct RootInfo
{
	// このルートの基準パス。各項目のパス復元時に使う。
	std::filesystem::path path;
	// 所属するワークスペースの番号。
	uint32_t workspaceIndex{0};
};

/**
	項目IDからルート相対パスを復元する
	@param[in] id 項目ID
	@param[in] entries 項目一覧
	@param[in] strings 文字列プール
	@return 復元した相対パス。不正なIDまたは親子関係の場合は空のパス
	*/
inline
std::filesystem::path BuildRelativePath(
	EntryId id,
	const std::vector<Entry>& entries,
	const StringPool& strings
)
{
	if (id >= entries.size()) return {};
	const RootId rootId = entries[id].rootId;
	std::vector<StringId> names;
	EntryId currentId = id;
	while (currentId != InvalidEntryId) {
		if (currentId >= entries.size() || names.size() >= entries.size()) return {};
		const Entry& entry = entries[currentId];
		if (entry.rootId != rootId || strings.Get(entry.nameId).empty()) return {};
		names.push_back(entry.nameId);
		currentId = entry.parentId;
	}
	std::reverse(names.begin(), names.end());
	std::filesystem::path result;
	for (const StringId nameId : names) {
		result /= std::wstring(strings.Get(nameId));
	}
	return result;
}

/**
	項目IDからフルパスを復元する
	@param[in] id 項目ID
	@param[in] entries 項目一覧
	@param[in] roots ルート情報一覧
	@param[in] strings 文字列プール
	@return 復元したフルパス。不正なIDまたは親子関係の場合は空のパス
	*/
inline
std::filesystem::path BuildFullPath(
	EntryId id,
	const std::vector<Entry>& entries, const std::vector<RootInfo>& roots,
	const StringPool& strings
)
{
	if (id >= entries.size()) return {};

	const RootId rootId = entries[id].rootId;
	if (rootId >= roots.size()) return {};

	const auto relative = BuildRelativePath(id, entries, strings);
	if (relative.empty()) return {};

	return roots[rootId].path / relative;
}

} // namespace workspaceplugin

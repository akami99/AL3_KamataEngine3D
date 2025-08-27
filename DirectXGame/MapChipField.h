#pragma once
#include "KamataEngine.h"

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

/// <summary>
/// マップチップフィールド
/// </summary>
class MapChipField {
private:


	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// ブロックの個数
	static inline const uint32_t kNumBlockVertical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	MapChipData mapChipData_;

public:
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	// 範囲矩形
	struct Rect {
		float left = 0.0f;     // 左端
		float right = 0.0f;    // 右端
		float bottom = 0.0f;   // 下端
		float top = 0.0f;      // 上端
	};

private:
	void ResetMapChipData();

public:
	MapChipField();
	~MapChipField();

	// 1ブロックのサイズを取得
	float GetChipSize() const {
		return kBlockWidth;
	}

	uint32_t GetNumBlockVertical() const {
		return kNumBlockVertical;
	}

	uint32_t GetNumBlockHorizontal() const {
		return kNumBlockHorizontal;
	}

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);
};


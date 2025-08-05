#pragma once

#include "KamataEngine.h"

/// <summary>
/// タイトルシーン
/// </summary>
class TitleScene {
private:
	// 3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* modelTitleName_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::WorldTransform worldTransformTitle_;

	float counter_ = 0.0f;
	static inline const float kDuration = 1.0f;
	float returnT_ = 1.0f;
	static inline const float kOriginePos = 1.4f;

	// カメラ
	KamataEngine::Camera camera_;

	// 終了フラグ
	bool finished_ = false;
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	~TitleScene();

	/// <summary>
	/// 更新
	/// <summary>
	void Update();

	/// <summary>
	/// 描画
	/// <summary>
	void Draw();

	// デスフラグのgetter
	bool IsFinished() const {
		return finished_;
	}
};


#pragma once

#include "KamataEngine.h"
#include "Skydome.h"
#include "Fade.h"

/// <summary>
/// ゲームクリアシーン
/// </summary>
class GameClearScene {
private:
	// シーンのフェーズ
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン部
		kFadeOut, // フェードアウト
	};

	// 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

	// 3Dモデルデータ
	KamataEngine::Model* modelClearName_ = nullptr;
	KamataEngine::Model* modelSkydome_ = nullptr;
	KamataEngine::Model* modelBackGround_ = nullptr;

	KamataEngine::WorldTransform worldTransformClear_;
	KamataEngine::WorldTransform worldTransformBackGround1_;
	KamataEngine::WorldTransform worldTransformBackGround2_;
	KamataEngine::WorldTransform worldTransformBackGround3_;

	float counter_ = 0.0f;
	static inline const float kDuration = 1.0f;
	float returnT_ = 1.0f;
	static inline const float kOriginePos = 1.4f;

	Fade* fade_ = nullptr;
	static inline const float kFadeTime = 2.0f;

	// 天球
	Skydome* skydome_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	// 終了フラグ
	bool finished_ = false;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	~GameClearScene();

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

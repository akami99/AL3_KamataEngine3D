#pragma once

#include "KamataEngine.h"
#include "Skydome.h"
#include "Fade.h"
#include "Tutorial.h"
#include <vector>

/// <summary>
/// ステージ選択シーン
/// </summary>
class StageSelectScene {
private:
	// シーンのフェーズ
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // 操作可能
		kFadeOut, // フェードアウト
	};

	// 演出用の定数
	static inline const float kIconGap = 2.0f;        // アイコンの間隔
	static inline const float kBaseRotSpeed = 0.02f;  // 通常時の回転速度
	static inline const float kSelectRotSpeed = 0.5f; // 選択瞬間の回転速度
	static inline const float kRotAttenuation = 0.1f; // 回転の減衰率

public:
	~StageSelectScene();
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// 終了フラグの取得
	bool IsFinished() const { return finished_; }

	// 選択されたステージ番号を取得（1～）
	int GetSelectedStageNo() const { return currentStageIndex_ + 1; }

public:
	// 現在選択中のインデックス（staticにすることでシーンを作り直しても記憶する）
	static int currentStageIndex_;

private:
	// メンバ変数
	Phase phase_ = Phase::kFadeIn;
	bool finished_ = false;

	// フェード
	Fade* fade_ = nullptr;

	// 操作方法
	Tutorial* tutorial_ = nullptr;

	// テクスチャハンドル
	uint32_t tutorialButtonHandle_ = 0;

	// 2Dテクスチャ
	KamataEngine::Sprite* spriteTutorialButton_ = nullptr;

	// モデル
	KamataEngine::Model* modelIcon_ = nullptr;
	KamataEngine::Model* modelSkydome_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	// アイコン（ステージごとのワールド変換）
	std::vector<KamataEngine::WorldTransform*> iconTransforms_;

	// ステージ数
	static const int kStageCount = 3;


	// 回転アニメーション用変数
	float currentRotSpeed_ = kBaseRotSpeed;

	// 入力制御用（押しっぱなし防止）
	bool isLeftPressed_ = false;
	bool isRightPressed_ = false;
};

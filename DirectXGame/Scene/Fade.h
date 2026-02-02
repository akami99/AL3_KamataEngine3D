#pragma once

#include "kamataEngine.h"

/// <summary>
/// フェード
/// </summary>
class Fade {
public:

	// フェードの状態
	enum class Status {
		None,     // フェードなし
		FadeIn,   // フェードイン中
		FadeOut,  // フェードアウト中
	};

	// 現在のフェードの状態
	Status status_ = Status::None;

private:
	// フェードの持続時間
	float duration_ = 0.0f;
	// 経過時間カウンター
	float counter_ = 0.0f;

	KamataEngine::Sprite* sprite_ = nullptr;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	// デストラクタ
	~Fade();

	/// <summary>
	/// 更新
	/// <summary>
	void Update();

	/// <summary>
	/// 描画
	/// <summary>
	void Draw();

	// フェード開始
	void Start(Status status, float duration);

	// フェード終了
	void Stop();

	// フェード終了判定
	bool IsFinished() const;
};


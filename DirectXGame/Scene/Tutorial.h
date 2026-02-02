#pragma once
#include "KamataEngine.h"
#include <vector>
#include <string>
#include <cmath>

// チュートリアルの状態
enum class TutorialState {
	Hidden,     // 非表示
	Opening,    // 出現アニメーション
	Active,     // 表示中（操作待ち）
	Closing     // 終了アニメーション
};

class Tutorial {
public:
	// コンストラクタ・デストラクタ
	Tutorial();
	~Tutorial();

	// 初期化
	void Initialize();

	// 更新
	void Update(const KamataEngine::Camera& camera);

	// 描画
	void Draw(const KamataEngine::Camera& camera);

	// ■ 指定した番号のテキストを表示して開始
	void Open(int index = 0);

	// 強制終了
	void Close();

	// ■ モデル（ページ）を追加
	// modelName: "tutorial01" などのモデル名（Resourcesフォルダ内のフォルダ名）
	void AddPage(const std::string& modelName);

	// 表示中かどうか
	bool IsActive() const { return state_ != TutorialState::Hidden; }

private:
	// ページデータ（モデル）のリスト
	std::vector<KamataEngine::Model*> pageModels_;

	// 表示用のワールド変換（1つを使い回す）
	KamataEngine::WorldTransform worldTransform_;

	// 現在のページ番号
	int currentPageIndex_ = 0;

	// 状態管理
	TutorialState state_ = TutorialState::Hidden;

	// アニメーション管理
	float timer_ = 0.0f;
	const float kAnimTime_ = 0.5f; // アニメーションにかかる時間（秒）

	// 内部処理: 次のページへ
	void NextPage();

	// 内部処理: カメラの前に配置し、スケールを更新
	void UpdateTransform(float scaleRatio, const KamataEngine::Camera& camera);

	// イージング関数（簡易版）
	float EaseOutBack(float x);
	float EaseInBack(float x);
};
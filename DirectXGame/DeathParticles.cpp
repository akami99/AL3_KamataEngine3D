#include "DeathParticles.h"
#include "WorldTransform.h"
#include "EngineMathFunctions.h"
#include "MatrixGenerators.h"
#include <numbers>
#include <algorithm>

using namespace KamataEngine;

void DeathParticles::Initialize(Model* model, Camera* camera, const Vector3& position) {
#ifdef DEBUG

	// NULLポインタチェック
	assert(model);

#endif // DEBUG
	if (!model) {
		return; // または適切なエラーハンドリング
	}

	// 引数の内容をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	for (WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
	}

	objectColor_.Initialize();
	color_ = { 1, 1, 1, 1 };
}

void DeathParticles::Update() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// 移動
	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// 基本となる速度ベクトル
		Vector3 velocity = { kSpeed, 0, 0 };
		// 回転角を計算する
		float angle = kAngleUnit * i;
		// Z軸周り回転行列
		Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);
		// 基本ベクトルを回転させて速度ベクトルを得る
		velocity = TransformVector(velocity, matrixRotation);
		// 移動処理
		worldTransforms_[i].translation_ += velocity;
	}

	// カウンターを１フレーム分の秒数進める
	counter_ += 1.0f / 60.0f;

	// 存続時間の上限に達したら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		// 終了扱いにする
		isFinished_ = true;
	}

	// パーティクルの寿命に対する進行度を0.0f〜1.0fで計算
	float t = counter_ / kDuration;

	color_.w = std::clamp((1.0f - t), 0.0f, 1.0f);
	// 色変更オブジェクトに色の数値を設定する
	objectColor_.SetColor(color_);

	for (WorldTransform& worldTransform : worldTransforms_) {
		// 行列計算
		UpdateWorldTransform(worldTransform);
	}
}

void DeathParticles::Draw() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// 3Dモデルを描画
	for (WorldTransform& worldTransform : worldTransforms_) {
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
}
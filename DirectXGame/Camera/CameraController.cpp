#define NOMIMAX

#include "CameraController.h"
#include "KamataEngine.h"
#include "EngineMathFunctions.h"
#include "Player/Player.h"
#include <numbers>
#include <algorithm> // std::clamp

using namespace KamataEngine;

CameraController::CameraController(KamataEngine::Camera& camera) : camera_(camera) {
}

void CameraController::Initialize() {
	// 移動範囲の指定
	movableArea_ = kSetMovableArea;
	// 見下ろし型にする
	camera_.rotation_ = { std::numbers::pi_v<float> / 4, 0.0f, 0.0f };
}

void CameraController::Update() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	if (target_->IsCameraStop()) {
		// 追従対象の座標は更新するがカメラの座標は更新しない
		Vector3 targetVelocity = target_->GetVelocity();
		targetPosition_ = targetWorldTransform.translation_ + TargetOffset_;

	} else {
		// --- 目標座標の計算 ---
		Vector3 targetVelocity = target_->GetVelocity();

		float velocityBias = 0.0f;
		targetPosition_ = targetWorldTransform.translation_ + TargetOffset_ + targetVelocity * velocityBias;

		// --- 座標補間（Lerp）によりゆったり追従 ---
		// 目安: 0.1f (ゆったり) ～ 0.2f (キビキビ)
		float interpRate = 0.1f; // 元の kInterpolationRate が大きすぎる場合は下げる

		camera_.translation_.x = Lerp(camera_.translation_.x, targetPosition_.x, interpRate);

		camera_.translation_.y = targetPosition_.y;

		camera_.translation_.z = Lerp(camera_.translation_.z, targetPosition_.z, interpRate);
	}

	// --- 移動範囲制限（ステージ端など） ---
	camera_.translation_.x = std::clamp(camera_.translation_.x, movableArea_.left, movableArea_.right);
	camera_.translation_.z = std::clamp(camera_.translation_.z, movableArea_.bottom, movableArea_.top);

	// 行列を更新する
	camera_.UpdateMatrix();
}

void CameraController::Reset() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットからカメラの位置を計算
	camera_.translation_ = targetWorldTransform.translation_ + TargetOffset_;
}
#define NOMIMAX

#include "CameraController.h"
#include "KamataEngine.h"
#include "EngineMathFunctions.h"
#include "Player.h"
#include <numbers>
#include <algorithm> // std::clamp

using namespace KamataEngine;

CameraController::CameraController(KamataEngine::Camera& camera) : camera_(camera) {
}

void CameraController::Initialize() {
	// 移動範囲の指定
	movableArea_ = { 11, 88, 6, 13 };
	// 見下ろし型にする
	camera_.rotation_ = { std::numbers::pi_v<float> / 4, 0.0f, 0.0f };
}

void CameraController::Update() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	Vector3 targetVelocity = target_->GetVelocity();
	targetPosition_ = targetWorldTransform.translation_ + TargetOffset_ + targetVelocity * kVelocityBias;

	// 座標補間によりゆったり追従
	camera_.translation_.x = targetPosition_.x;
	camera_.translation_.y = targetPosition_.y;
	camera_.translation_.z = targetPosition_.z;

	//// 追従対象が画面外に出ないように補正
	//camera_.translation_.x = (std::max)(camera_.translation_.x, targetPosition_.x + kMargin.left);
	//camera_.translation_.x = (std::min)(camera_.translation_.x, targetPosition_.x + kMargin.right);
	//camera_.translation_.y = (std::max)(camera_.translation_.y, targetPosition_.y + kMargin.bottom);
	//camera_.translation_.y = (std::min)(camera_.translation_.y, targetPosition_.y + kMargin.top);

	// 移動範囲制限
	camera_.translation_.x = std::clamp(camera_.translation_.x, movableArea_.left, movableArea_.right);
	camera_.translation_.y = std::clamp(camera_.translation_.y, movableArea_.bottom, movableArea_.top);
	
	// 行列を更新する
	camera_.UpdateMatrix();
}

void CameraController::Reset() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットからカメラの位置を計算
	camera_.translation_ = targetWorldTransform.translation_ + TargetOffset_;
}
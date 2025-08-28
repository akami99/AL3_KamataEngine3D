#define NOMIMAX

#include "Player.h"
#include "EngineMathFunctions.h"
#include "WorldTransform.h"
#include "MapChipField.h"
#include <cassert>
#include <numbers>
#include <algorithm>

using namespace KamataEngine;

void Player::Initialize(Model* model, Model* modelAttack, Camera* camera, const Vector3& position) {
#ifdef _DEBUG

	// NULLポインタチェック
	assert(model);

#endif // _DEBUG
	if (!model) {
		return; // または適切なエラーハンドリング
	}
	// 引数の内容をメンバ変数に記録
	model_ = model;
	modelAttack_ = modelAttack;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	worldTransformAttack_.Initialize();
	worldTransformAttack_.translation_ = position;
	worldTransformAttack_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player::Update() {
	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振る舞いを変更する
		behavior_ = behaviorRequest_;
		// 各振る舞いごとの初期化を実行

		switch (behavior_) {
			// 通常行動
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;
			// 攻撃行動
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		}
		// 振る舞いリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}
	switch (behavior_) {
		// 通常行動
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;
		// 攻撃行動
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	}

}

void Player::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);

	if (attackPhase_ == AttackPhase::Rush) {
		modelAttack_->Draw(worldTransformAttack_, *camera_);
	}
}

void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;
	isDead_ = true;
}

KamataEngine::Vector3 Player::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得（ワールド行列）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kWidth / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kWidth / 2.0f, worldPos.z + kWidth / 2.0f };

	return aabb;
}

void Player::BehaviorRootUpdate() {
//#ifdef _DEBUG
//
//	// 攻撃キーを押したら
//	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
//		// 攻撃ビヘイビアをリクエスト
//		behaviorRequest_ = Behavior::kAttack;
//	}
//
//#endif // _DEBUG

	// 1.移動入力
	Move();

	// 2.移動量を加味して衝突判定する
	// 衝突情報を初期化
	CollisionMapinfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.moveAmount_ = velocity_;

	// マップ衝突チェック
	MapCollisionCheck(collisionMapInfo);

	// 3.判定結果を反映して移動させる
	ApplyCollisionResult(collisionMapInfo);

	// 4.天井に接触している場合の処理
	OnCollisionCeiling(collisionMapInfo);

	// 5.壁に接触している場合の処理
	OnCollisionWall(collisionMapInfo);

	// 6.接地状態の切り替え
	OnGroundSwitch(collisionMapInfo);

	// 7.旋回制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = {
			std::numbers::pi_v<float> / 2.0f, // 右
			std::numbers::pi_v<float> *3.0f / 2.0f, // 左
		};
		// 状況に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		// 自キャラの角度を設定する
		worldTransform_.rotation_.y = EaseInOutLerpFloat(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}

	// 8.行列計算
	UpdateWorldTransform(worldTransform_);
}

void Player::BehaviorAttackUpdate() {
	// 攻撃ギミックの経過時間カウンターを更新
	attackParameter_ += 1.0f / 60.0f; // 1フレームごとに時間を加算

	CollisionMapinfo collisionMapInfo;
	Vector3 velocity{};
	static const Vector3 kAttackVelocity = { 2.0f, 0.0f, 0.0f }; // 突進速度を定数として定義

	switch (attackPhase_) {
	case AttackPhase::Charge:
	default:
	{
		// 溜め動作の更新
		// tは時間の進行度を0.0fから1.0fにする
		float t = std::clamp(attackParameter_ / kChargeTime, 0.0f, 1.0f);
		worldTransform_.scale_.z = EaseOutLerpFloat(1.0f, 0.3f, t); // 画像の指示に合わせてEaseOutに変更
		worldTransform_.scale_.y = EaseOutLerpFloat(1.0f, 1.6f, t); // 画像の指示に合わせてEaseOutに変更

		// 前進動作へと移行
		if (attackParameter_ >= kChargeTime) {
			attackPhase_ = AttackPhase::Rush;
			attackParameter_ = 0.0f; // カウンターリセット
		}
	}
	break;
	case AttackPhase::Rush:
	{
		// 突進動作の更新
		// tは時間の進行度を0.0fから1.0fにする
		float t = std::clamp(attackParameter_ / kRushTime, 0.0f, 1.0f);
		worldTransform_.scale_.z = EaseOutLerpFloat(0.3f, 1.3f, t); // 画像の指示に合わせてEaseOutに変更
		worldTransform_.scale_.y = EaseInLerpFloat(1.6f, 0.7f, t); // 画像の指示に合わせてEaseInに変更

		// 突進時のみ移動を限定する
		if (lrDirection_ == LRDirection::kRight) {
			velocity = kAttackVelocity;
		} else {
			velocity = -1.0f * kAttackVelocity;
		}

		// 余韻動作へと移行
		if (attackParameter_ >= kRushTime) {
			attackPhase_ = AttackPhase::Aftertaste;
			attackParameter_ = 0.0f; // カウンターリセット
		}
	}
	break;
	case AttackPhase::Aftertaste:
	{
		// 余韻動作の更新
		// tは時間の進行度を0.0fから1.0fにする
		float t = std::clamp(attackParameter_ / kAftertasteTime, 0.0f, 1.0f);
		worldTransform_.scale_.z = EaseOutLerpFloat(1.3f, 1.0f, t); // スケールを通常に戻す
		worldTransform_.scale_.y = EaseOutLerpFloat(0.7f, 1.0f, t); // スケールを通常に戻す

		// ルート動作へと移行
		if (attackParameter_ >= kAftertasteTime) {
			attackPhase_ = AttackPhase::Charge; // 次の攻撃のためにChargeに戻す
			attackParameter_ = 0.0f; // カウンターリセット
			behaviorRequest_ = Behavior::kRoot; // ルートビヘイビアに戻る
		}
	}
	break;
	}

	// 衝突情報を初期化して、velocityを代入
	collisionMapInfo.moveAmount_ = velocity;

	// ここでUpdateWorldTransformに衝突情報と移動量を渡す
	// 実際のKamataEngineの関数はこれとは異なる可能性があります。
	// Player::Update()内で移動処理を適用している場合、この下の行は不要かもしれません。
	worldTransform_.translation_ += collisionMapInfo.moveAmount_;


	// トランスフォームの値をコピー
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	if (lrDirection_ == LRDirection::kRight) {
		worldTransformAttack_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	} else {
		worldTransformAttack_.rotation_.y = 3.0f * (std::numbers::pi_v<float> / 2.0f);
	}


	UpdateWorldTransform(worldTransform_);
	UpdateWorldTransform(worldTransformAttack_);
}

void Player::BehaviorRootInitialize() {}

void Player::BehaviorAttackInitialize() {
	// カウンター初期化
	attackParameter_ = 0;
}

void Player::Move() {
	// 移動入力
	// 接地状態
	if (onGround_) {
		// 左右移動操作
		if (Input::GetInstance()->PushKey(DIK_D) ||
			Input::GetInstance()->PushKey(DIK_A)) {

			// 左右加速
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_D)) {
				// 左入力中の右入力
				if (velocity_.x < 0.0f) {
					// 速度と逆方向中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x += kAcceleration;

				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_A)) {
				// 右入力中の左入力
				if (velocity_.x > 0.0f) {
					// 速度と逆方向中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x -= kAcceleration;

				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}
			// 加速/減速
			velocity_ += acceleration;

			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			// 非入力時は移動減衰を書ける
			velocity_.x *= (1.0f - kAttenuation);
		}
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			// ジャンプ初速
			velocity_ += Vector3(0.0f, kJumpAcceleration, 0.0f);
		}
		// 空中
	} else {
		// 落下速度
		velocity_ += Vector3(0.0f, -kGravityAcceleration, 0.0f);
		// 落下速度制限
		velocity_.y = (std::max)(velocity_.y, -kLimitFallSpeed);
	}
#ifdef _DEBUG

	if (Input::GetInstance()->PushKey(DIK_UP)) {
		// ジャンプ初速
		velocity_ += Vector3(0.0f, kJumpAcceleration, 0.0f);
	}

#endif // DEBUG

	// ***ここを削除してください***
	// worldTransform_.translation_ += velocity_;
}

void Player::MapCollisionCheck(CollisionMapinfo& info) {
	MapCollisionCheckUp(info);
	MapCollisionCheckDown(info);
	MapCollisionCheckRight(info);
	MapCollisionCheckLeft(info);
}

void Player::MapCollisionCheckUp(CollisionMapinfo& info) {
	// 上昇有り？
	if (info.moveAmount_.y <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
		CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

	//MapChipType mapChipType;
	// 真上の当たり判定を行う
	bool hit = false;
	// 初期化を追加
	MapChipField::IndexSet indexSetLeftTop = MapChipField::IndexSet(), indexSetRightTop = MapChipField::IndexSet();

	// 複数のブロックをまとめてチェックする
	for (float y = 0.0f; y <= info.moveAmount_.y; y += mapChipField_->GetChipSize()) {
		Vector3 checkPosLeftTop = positionNew[static_cast<uint32_t>(Corner::kLeftTop)] - Vector3(0, y, 0);
		Vector3 checkPosRightTop = positionNew[static_cast<uint32_t>(Corner::kRightTop)] - Vector3(0, y, 0);

		indexSetLeftTop = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeftTop);
		indexSetRightTop = mapChipField_->GetMapChipIndexSetByPosition(checkPosRightTop);

		if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftTop.xIndex, indexSetLeftTop.yIndex) == MapChipType::kBlock ||
			mapChipField_->GetMapChipTypeByIndex(indexSetRightTop.xIndex, indexSetRightTop.yIndex) == MapChipType::kBlock) {
			hit = true;
			break;
		}
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSetLeftTop.xIndex, indexSetLeftTop.yIndex);
		info.moveAmount_.y = rect.bottom - (worldTransform_.translation_.y + kHeight / 2.0f);
		// 天井に当たったことを記録する
		info.onCollisionCeiling_ = true;
	}
}

void Player::MapCollisionCheckDown(CollisionMapinfo& info) {
	// 下降あり？
	if (info.moveAmount_.y >= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
		CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

	//MapChipType mapChipType;
	// 真下の当たり判定を行う
	bool hit = false;
	// 初期化を追加
	MapChipField::IndexSet indexSetLeftBottom = MapChipField::IndexSet(), indexSetRightBottom = MapChipField::IndexSet();

	// 複数のブロックをまとめてチェックする
	for (float y = 0.0f; y >= info.moveAmount_.y; y -= mapChipField_->GetChipSize()) {
		Vector3 checkPosLeftBottom = positionNew[static_cast<uint32_t>(Corner::kLeftBottom)] - Vector3(0, y, 0);
		Vector3 checkPosRightBottom = positionNew[static_cast<uint32_t>(Corner::kRightBottom)] - Vector3(0, y, 0);

		indexSetLeftBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeftBottom);
		indexSetRightBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosRightBottom);

		if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex) == MapChipType::kBlock ||
			mapChipField_->GetMapChipTypeByIndex(indexSetRightBottom.xIndex, indexSetRightBottom.yIndex) == MapChipType::kBlock) {
			hit = true;
			break;
		}
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex);
		info.moveAmount_.y = rect.top - (worldTransform_.translation_.y - kHeight / 2.0f);
		// 地面に当たったことを記録する
		info.onCollisionGround_ = true;
	}
}

void Player::MapCollisionCheckRight(CollisionMapinfo& info) {
	// 右移動あり？
	if (info.moveAmount_.x <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
		CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

	//MapChipType mapChipType;
	// 右側の当たり判定を行う
	bool hit = false;
	// 初期化を追加
	MapChipField::IndexSet indexSetRightBottom = MapChipField::IndexSet(), indexSetRightTop = MapChipField::IndexSet();

	// 複数のブロックをまとめてチェックする
	for (float x = 0.0f; x <= info.moveAmount_.x; x += mapChipField_->GetChipSize()) {
		Vector3 checkPosRightBottom = positionNew[static_cast<uint32_t>(Corner::kRightBottom)] - Vector3(x, 0, 0);
		Vector3 checkPosRightTop = positionNew[static_cast<uint32_t>(Corner::kRightTop)] - Vector3(x, 0, 0);

		indexSetRightBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosRightBottom);
		indexSetRightTop = mapChipField_->GetMapChipIndexSetByPosition(checkPosRightTop);

		if (mapChipField_->GetMapChipTypeByIndex(indexSetRightBottom.xIndex, indexSetRightBottom.yIndex) == MapChipType::kBlock ||
			mapChipField_->GetMapChipTypeByIndex(indexSetRightTop.xIndex, indexSetRightTop.yIndex) == MapChipType::kBlock) {
			hit = true;
			break;
		}
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSetRightTop.xIndex, indexSetRightTop.yIndex);
		info.moveAmount_.x = rect.left - (worldTransform_.translation_.x + kWidth / 2.0f);
		// 壁に当たったことを記録する
		info.onCollisionWall_ = true;
	}
}

void Player::MapCollisionCheckLeft(CollisionMapinfo& info) {
	// 左移動あり？
	if (info.moveAmount_.x >= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
		CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

	//MapChipType mapChipType;
	// 左側の当たり判定を行う
	bool hit = false;
	// 初期化を追加
	MapChipField::IndexSet indexSetLeftBottom = MapChipField::IndexSet(), indexSetLeftTop = MapChipField::IndexSet();

	// 複数のブロックをまとめてチェックする
	for (float x = 0.0f; x >= info.moveAmount_.x; x -= mapChipField_->GetChipSize()) {
		Vector3 checkPosLeftBottom = positionNew[static_cast<uint32_t>(Corner::kLeftBottom)] - Vector3(x, 0, 0);
		Vector3 checkPosLeftTop = positionNew[static_cast<uint32_t>(Corner::kLeftTop)] - Vector3(x, 0, 0);

		indexSetLeftBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeftBottom);
		indexSetLeftTop = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeftTop);

		if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex) == MapChipType::kBlock ||
			mapChipField_->GetMapChipTypeByIndex(indexSetLeftTop.xIndex, indexSetLeftTop.yIndex) == MapChipType::kBlock) {
			hit = true;
			break;
		}
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSetLeftTop.xIndex, indexSetLeftTop.yIndex);
		info.moveAmount_.x = (worldTransform_.translation_.x - kWidth / 2.0f) - rect.right;
		// 壁に当たったことを記録する
		info.onCollisionWall_ = true;
	}
}

Vector3 Player::CornerPosition(Vector3 center, Corner corner) {
	Vector3 offsetTable[static_cast<uint32_t>(Corner::kNumCorner)] = {
		Vector3{+kWidth / 2.0f, -kHeight / 2.0f, 0}, // kRightBottom
		Vector3{-kWidth / 2.0f, -kHeight / 2.0f, 0}, // kLeftBottom
		Vector3{+kWidth / 2.0f, +kHeight / 2.0f, 0}, // kRightTop
		Vector3{-kWidth / 2.0f, +kHeight / 2.0f, 0}, // kLeftTop
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

std::array<Vector3, static_cast<uint32_t>(Player::Corner::kNumCorner)> Player::CalculateCornerPositions(const Vector3& basePosition) {
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positions;

	for (uint32_t i = 0; i < positions.size(); ++i) {
		// CornerPositionはbasePositionと、各コーナーの種類を受け取る
		positions[i] = CornerPosition(basePosition, static_cast<Corner>(i));
	}
	return positions;
}

void Player::ApplyCollisionResult(const CollisionMapinfo& info) {
	// 移動
	worldTransform_.translation_ += info.moveAmount_;
}

void Player::OnCollisionCeiling(const CollisionMapinfo& info) {
	// 天井に当たった？
	if (info.onCollisionCeiling_) {
		velocity_.y = 0.0f;
	}
}

void Player::OnCollisionWall(const CollisionMapinfo& info) {
	// 壁に当たった？
	if (info.onCollisionWall_) {
		// 速度をゼロにする
		velocity_.x = 0.0f;
	}
}

void Player::OnGroundSwitch(const CollisionMapinfo& info) {
	// 自キャラが接地状態？
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;
		} else {
			// 移動後の4つの角の座標
			std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
				CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

			//MapChipType mapChipType;
			// 真下の当たり判定を行う
			bool hit = false;
			MapChipField::IndexSet indexSetLeftBottom = MapChipField::IndexSet(), indexSetRightBottom = MapChipField::IndexSet();

			// 複数のブロックをまとめてチェックする
			for (float y = 0.0f; y >= info.moveAmount_.y; y -= mapChipField_->GetChipSize()) {
				Vector3 checkPosLeftBottom = positionNew[static_cast<uint32_t>(Corner::kLeftBottom)] - Vector3(0, y, 0);
				Vector3 checkPosRightBottom = positionNew[static_cast<uint32_t>(Corner::kRightBottom)] - Vector3(0, y, 0);

				indexSetLeftBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeftBottom);
				indexSetRightBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosRightBottom);

				if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex) == MapChipType::kBlock ||
					mapChipField_->GetMapChipTypeByIndex(indexSetRightBottom.xIndex, indexSetRightBottom.yIndex) == MapChipType::kBlock) {
					hit = true;
					break;
				}
			}

			// ブロックにヒット？
			if (!hit) {
				onGround_ = false; // 接地状態を解除
			}
		}
	} else {
		if (info.onCollisionGround_) {
			// 着地状態に切り替える（落下を止める）
			onGround_ = true;
			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// Y速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}
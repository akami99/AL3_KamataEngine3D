#include "Stage.h"
#include "WorldTransform.h"
#include <cassert>
#include <numbers> // C++20 数学定数用
#include <fstream> // ファイル読み込み用
#include <iostream>

// JSONライブラリのインクルード
#include "externals/json.hpp"
using json = nlohmann::json;

using namespace KamataEngine;
Stage::~Stage() {
	// ブロックの解放
	for (auto* block : blocks_) {
		delete block;
	}
	blocks_.clear();

    if (floorTransform_) {
        delete floorTransform_;
        floorTransform_ = nullptr;
	}

	// 敵の解放
	for (auto* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

    // ドアの解放
    if (door_) {
        delete door_;
        door_ = nullptr;
    }

    // 自キャラの解放
    if (player_) {
        delete player_;
        player_ = nullptr;
    }
}

void Stage::Initialize(Model* modelPlayer, Model* modelAttack, Model* modelDoor, Model* modelBlock, Model* modelEnemy, Model* modelEnemyBullet) {
    modelPlayer_ = modelPlayer;
    modelAttack_ = modelAttack;
    modelDoor_ = modelDoor;
    modelBlock_ = modelBlock;
	modelEnemy_ = modelEnemy;
	modelEnemyBullet_ = modelEnemyBullet;
	// ここではリストの初期化はせず、Load関数で行います
}

void Stage::Update() {

    if (!player_->IsDead()) {
        // 自キャラの更新
        player_->Update();

        // ゴールのドアの更新
        door_->Update();

        // 全ての敵を更新
        for (Enemy* enemy : enemies_) {
            if (enemy->IsDead()) {
                continue; // 死んでいる敵はスキップ
            }

            // プレイヤー情報をセット（毎フレームあるいは初期化時）
            enemy->SetPlayer(player_);
            enemy->Update();
        }
    }

	// 死んだ敵をリストから削除する処理などもここに書けます
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});
}

void Stage::Load(const std::string& fileName) {
	// 1. ファイルを開く
	std::ifstream file("Resources/stages/" + fileName); // フォルダ構成に合わせて調整
	if (!file.is_open()) {
		// ファイルが開けなかった場合のエラーメッセージ（デバッグ用）
		assert(false && "stage.json not found!");
		return;
	}

	// 2. JSONをパース（解析）する
	json deserialized;
	file >> deserialized;

	// 3. "objects" という配列の中身を一つずつ処理する
	// (jsonファイルの中身が "objects": [ ... ] となっている前提)
	if (deserialized.contains("objects") && deserialized["objects"].is_array()) {

        for (auto& object : deserialized["objects"]) {
            // "type" が無いデータ（ダミーなど）は飛ばす
            if (!object.contains("type")) continue;

            std::string type = object["type"];

            // -------------------------------------------------
            // 座標 (translation) の読み取り
            // -------------------------------------------------
            Vector3 pos = { 0, 0, 0 };
            if (object.contains("translation")) {
                pos.x = (float)object["translation"][0];
                pos.y = (float)object["translation"][1];
                pos.z = (float)object["translation"][2];
            }

            // -------------------------------------------------
            // サイズ (scaling) の読み取り
            // -------------------------------------------------
            Vector3 scale = { 1, 1, 1 };
            if (object.contains("scaling")) {
                scale.x = (float)object["scaling"][0];
                scale.y = (float)object["scaling"][1];
                scale.z = (float)object["scaling"][2];
            }

            // -------------------------------------------------
            // 回転 (rotation) の読み取り
            // ※JSONは度数法(Degree)で保存したので、ラジアンに戻す
            // -------------------------------------------------
            Vector3 rot = { 0, 0, 0 };
            if (object.contains("rotation")) {
                float degX = (float)object["rotation"][0];
                float degY = (float)object["rotation"][1];
                float degZ = (float)object["rotation"][2];

                // 度数法 -> ラジアン変換
                rot.x = degX * (std::numbers::pi_v<float> / 180.0f);
                rot.y = degY * (std::numbers::pi_v<float> / 180.0f);
                rot.z = degZ * (std::numbers::pi_v<float> / 180.0f);
            }

            // -------------------------------------------------
            // 種類ごとの生成処理
            // -------------------------------------------------

            // ★プレイヤー (PLAYER)
            if (type == "PLAYER") {
                // すでにプレイヤーがいる場合は生成しない（重複防止）
                if (player_ == nullptr) {
                    player_ = new Player();
                    // 自キャラの初期化 (読み込んだ座標 pos を渡す)
                    player_->Initialize(modelPlayer_, modelAttack_, pos);
                }
            }

            // ★ゴール (DOOR)
            else if (type == "DOOR") {
                if (door_ == nullptr) {
                    door_ = new Door();
                    // ドアの初期化 (読み込んだ座標 pos を渡す)
                    door_->Initialize(modelDoor_, pos);
                }
            }

            // ★敵 (ENEMY)
            if (type == "ENEMY") {
                Enemy* newEnemy = new Enemy();
                // 敵のタイプ読み込み（デフォルトはWALK）
                Enemy::Type enemyType = Enemy::Type::kWark;
                if (object.contains("param") && object["param"] == "SHOOT") {
                    enemyType = Enemy::Type::kShoot;
                }

                // 初期化してリストに追加
                newEnemy->Initialize(modelEnemy_, pos, enemyType);
				newEnemy->SetBulletModel(modelEnemyBullet_); // 敵の弾に使うモデルをセット
                enemies_.push_back(newEnemy);
            }

            // ★ブロック (BLOCK)
            else if (type == "BLOCK") {
                WorldTransform* newBlock = new WorldTransform();
                newBlock->Initialize();
                newBlock->translation_ = pos;
                newBlock->scale_ = scale;
                newBlock->rotation_ = rot;
                UpdateWorldTransform(*newBlock); // 行列更新

                blocks_.push_back(newBlock);
            }

            // ★床 (FLOOR)
            else if (type == "FLOOR") {
                // 床はリストではなく単体の変数 floorTransform_ なので個別に処理
                if (floorTransform_ == nullptr) {
                    floorTransform_ = new WorldTransform();
                    floorTransform_->Initialize();
                }
                floorTransform_->translation_ = pos;
                floorTransform_->scale_ = scale;
                floorTransform_->rotation_ = rot;
                UpdateWorldTransform(*floorTransform_);
            }
        }
    }

	file.close();

    // 生成されたプレイヤーを、全ての敵にセットする
    if (player_) {
        for (Enemy* enemy : enemies_) {
            // プレイヤー情報をセット
            enemy->SetPlayer(player_);
        }
    }
}

void Stage::Draw(const Camera& camera) {
	// ブロックの描画
	for (WorldTransform* block : blocks_) {
		// 描画
		modelBlock_->Draw(*block, camera);
	}
	// 床の描画
    if (floorTransform_) {
        // 描画
        modelBlock_->Draw(*floorTransform_, camera);
	}

    // ドアの描画
    if (door_) {
        door_->Draw(camera);
    }
    
    // プレイヤーの描画
    if (!player_->IsDead()) {
        player_->Draw(camera);
    }

	// 敵の描画
	for (Enemy* enemy : enemies_) {
		enemy->Draw(camera);
	}
}
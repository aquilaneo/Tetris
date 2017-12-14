#pragma once
#include"game.h"
#include<iostream>

#define PI 3.14159265358979323846

using namespace std;

namespace Tetris {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	const int field_width = 10;
	const int field_height = 20;
	const int blk_count = 4;
	int interval = 500;


	// ---------- Class to manage positions ----------
	ref class Position {
	public:
		double x, y;

		Position () {
			x = 0.0;
			y = 0.0;
		}
		Position (double set_x, double set_y) {
			x = set_x;
			y = set_y;
		}
		Position (const Position% pos) {
			x = pos.x;
			y = pos.y;
		}
	};
	// ---------------------------------------------------

	// ---------- Class to manage blocks ----------
	ref class Block {
		// 各ブロックの絶対座標を求める
		void Rel2Abs (Position^ rel_pos, Position^ abs_pos, Position^ orig);
		int angle = 0;
	public:
		static Block^ controling;

		Position orig_position; // 基準座標
		Position center_position; // 中心座標 (基準座標からの相対座標)
		cli::array<Position^>^ rel_positions = gcnew cli::array<Position^> (blk_count); // 各ブロックの、基準座標からの相対座標
		cli::array<Position^>^ rot_positions = gcnew cli::array<Position^> (blk_count); // 各ブロックの、回転後の相対座標
		cli::array<Position^>^ abs_positions = gcnew cli::array<Position^> (blk_count); // 各ブロックの絶対座標
		int type; // ブロックのタイプ　(1~7)

		Block (int set_type, Position set_position);
		property Position Orig_Position {
			void set (Position set_pos) {
				// 絶対座標も一緒に計算する
				orig_position.x = set_pos.x;
				orig_position.y = set_pos.y;

				for (int i = 0; i < blk_count; i++) {
					Rel2Abs (rel_positions [i], abs_positions [i], %orig_position);
				}
			}
			Position get () {
				return orig_position;
			}
		}
		bool Move (Position move_vec);
		bool Rotate (int direction);
	};
	// ------------------------------------------------

	// ---------- Class to manage squares ----------
	ref class Square {
		PictureBox^ box;
	public:
		Block^ block; // このマスにあるブロック

		// 初期化
		Square (Control::ControlCollection^ controls, int x, int y, int width, int height) {
			box = gcnew PictureBox ();
			box->Location = Point (x, y);
			box->Size = Size (width, height);
			Set_Block (nullptr);
			controls->Add (box);
		}

		// ブロックをこのマスにセット
		void Set_Block (Block^ set_block);
	};
	// -------------------------------------------------

	// ---------- Static class to manage this game ----------
	static ref class Game {
		static Random^ r;
		static Label^ label_ptr;
		static int score = 0;

		// Game over
		static void GameOver () {
			controlable_flg = false;
			Show_Label ("Game Over...");
		}
	public:
		static cli::array<Square ^, 2>^ squares;
		static bool controlable_flg = false;

		// Start the game
		static void Start (Control::ControlCollection^ controls, Label^ label) {
			r = gcnew Random ();
			label_ptr = label;

			squares = gcnew cli::array<Square^, 2> (field_width, field_height);
			for (int x = 0; x < field_width; x++) {
				for (int y = 0; y < field_height; y++) {
					squares [x, y] = gcnew Square (controls, 1 + 30 * x, 1 + 30 * y, 28, 28);
				}
			}
			controlable_flg = true;
		}
		// Add new block
		static bool Game::Spawn_Block () {
			int type = r->Next (1, 8);

			Position new_pos (4, 1);
			Block^ spawn_block = gcnew Block (type, new_pos);

			// Game area check
			if (!Check_Field (spawn_block->abs_positions, spawn_block)) {
				GameOver ();
				return false;
			}

			for (int i = 0; i < blk_count; i++) { // ブロックがのっかっているマスにこのブロックを記録
				squares [(int) spawn_block->abs_positions [i]->x, (int) spawn_block->abs_positions [i]->y]->Set_Block (spawn_block);
			}

			return true;
		}

		// Check collision with field walls and other blocks
		static bool Check_Field (cli::array<Position^>^ position, Block^ block_ptr) {
			for (int i = 0; i < blk_count; i++) {
				double x = position [i]->x;
				double y = position [i]->y;

				// Game area check
				if (x < 0.0 || x >= field_width || y < 0.0 || y >= field_height) {
					return false;
				}
				// Other block check
				if (Game::squares [(int) x, (int) y]->block != block_ptr && Game::squares [(int) x, (int) y]->block != nullptr) {
					return false;
				}
			}
			return true;
		}

		// Check if the line is filled
		static void Check_Fill () {
			for (int y = field_height - 1; y >= 0; y--) {
				int fill_count = 0;
				// Filled check
				for (int x = 0; x < field_width; x++) {
					if (squares [x, y]->block != nullptr) {
						fill_count++;
					} else {
						break;
					}
				}
				// Clear line
				if (fill_count == field_width) {
					Game::controlable_flg = false;
					for (int x = 0; x < field_width; x++) {
						squares [x, y]->Set_Block (nullptr);
						Wait (10);
					}
					Game::Add_Score ();

					Wait (100);


					// Fall blocks
					for (int i = y; i >= 1; i--) {
						for (int x = 0; x < field_width; x++) {
							Block^ fall_blk = squares [x, i - 1]->block;
							squares [x, i]->Set_Block (fall_blk);
						}
					}
					y++;

					// Shorten interval to fall block
					interval -= 20;
					if (interval < 100) {
						interval = 100;
					}

					Game::controlable_flg = true;
				}
			}
		}

		// Check if own_block is accumulated
		static bool Check_Accumulate (Block^ own_block, cli::array<Position^>^ abs_positions) {
			// 一番下ついた判定
			for (int i = 0; i < blk_count; i++) {
				int x = (int) abs_positions [i]->x;
				int y = (int) abs_positions [i]->y;
				if (y == field_height - 1 ||
					Game::squares [x, y + 1]->block != nullptr && Game::squares [x, y + 1]->block != own_block) {
					return true;
				}
			}
			return false;
		}

		// Add score
		static void Add_Score () {
			score += 100;
			Console::WriteLine ("Score : " + score);
		}

		// Show text with label
		static void Show_Label (string text) {
			label_ptr->Text = gcnew String (text.c_str ());
		}

		// Wait process
		static void Wait (int milliseconds) {
			controlable_flg = false;
			Application::DoEvents ();
			System::Threading::Thread::Sleep (milliseconds);
			controlable_flg = true;
		}
	};
	// -----------------------------------------------------------


	/// <summary>
	/// MyForm の概要
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form {
	public:
		MyForm (void) {
			InitializeComponent ();
			//
			//TODO: ここにコンストラクター コードを追加します
			//
		}

	protected:
		/// <summary>
		/// 使用中のリソースをすべてクリーンアップします。
		/// </summary>
		~MyForm () {
			if (components) {
				delete components;
			}
		}
	private: System::ComponentModel::IContainer^  components;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Button^  button1;
	protected:
	private: System::Windows::Forms::Timer^  timer1;

	protected:

	private:
		/// <summary>
		/// 必要なデザイナー変数です。
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// デザイナー サポートに必要なメソッドです。このメソッドの内容を
		/// コード エディターで変更しないでください。
		/// </summary>
		void InitializeComponent (void) {
			this->components = (gcnew System::ComponentModel::Container ());
			this->timer1 = (gcnew System::Windows::Forms::Timer (this->components));
			this->label1 = (gcnew System::Windows::Forms::Label ());
			this->button1 = (gcnew System::Windows::Forms::Button ());
			this->SuspendLayout ();
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Interval = 500;
			this->timer1->Tick += gcnew System::EventHandler (this, &MyForm::timer1_Tick);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Font = (gcnew System::Drawing::Font (L"MS UI Gothic", 48, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(128)));
			this->label1->Location = System::Drawing::Point (29, 447);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size (0, 112);
			this->label1->TabIndex = 0;
			// 
			// button1
			// 
			this->button1->Font = (gcnew System::Drawing::Font (L"MS UI Gothic", 9.857143F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(128)));
			this->button1->Location = System::Drawing::Point (220, 447);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size (224, 75);
			this->button1->TabIndex = 1;
			this->button1->Text = L"Game Start";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler (this, &MyForm::button1_Click);
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF (11, 21);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size (685, 1404);
			this->Controls->Add (this->button1);
			this->Controls->Add (this->label1);
			this->Name = L"MyForm";
			this->Text = L"MyForm";
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler (this, &MyForm::MyForm_KeyDown);
			this->ResumeLayout (false);
			this->PerformLayout ();

		}
#pragma endregion
	private: System::Void MyForm_KeyDown (System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
		if (Game::controlable_flg) {
			Position move_vec (0, 0);
			switch (e->KeyData) {
				case Keys::Up:
					move_vec.y = -1;
					Block::controling->Move (move_vec);
					break;
				case Keys::Left:
					move_vec.x = -1;
					Block::controling->Move (move_vec);
					break;
				case Keys::Right:
					move_vec.x = 1;
					Block::controling->Move (move_vec);
					break;
				case Keys::A:
					Block::controling->Rotate (-1);
					break;
				case Keys::D:
					Block::controling->Rotate (1);
					break;
				case Keys::Space:
					move_vec.y = 1;
					Block::controling->Move (move_vec);
					break;
				default:
					return;
			}
		}
	}
	private: System::Void timer1_Tick (System::Object^  sender, System::EventArgs^  e) {
		if (Game::controlable_flg) {
			timer1->Interval = interval;
			if (Game::controlable_flg) {
				Position move_vec (0, 1);
				Block::controling->Move (move_vec);
			}
		}
	}
	private: System::Void button1_Click (System::Object^  sender, System::EventArgs^  e) {
		button1->Hide ();

		Game::Start (this->Controls, label1);

		Position pos (4, 1);
		Block^ block = gcnew Block (1, pos);
		Game::Spawn_Block ();

		this->Focus ();
	}
	};
}

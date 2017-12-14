#include "MyForm.h"

using namespace Tetris;

int main () {
	MyForm ^ form = gcnew MyForm ();
	form->ShowDialog ();
	return 0;
}

// ---------- Class to manage blocks ----------
Block::Block (int set_type, Position set_position) {
	Block::controling = this;

	type = set_type;
	orig_position.x = set_position.x;
	orig_position.y = set_position.y;
	center_position.x = 0;
	center_position.y = 0;

	cli::array<Position^, 2>^ blocks = gcnew cli::array<Position^, 2> {
		{gcnew Position (-1, 0), gcnew Position (0, 0), gcnew Position (1, 0), gcnew Position (2, 0)},
		{gcnew Position (-1, -1), gcnew Position (-1, 0), gcnew Position (0, 0), gcnew Position (1, 0)},
		{gcnew Position (-1,0), gcnew Position (0,0), gcnew Position (1,0), gcnew Position (1,-1)},
		{gcnew Position (-1,0),gcnew Position (0,0),gcnew Position (0,-1),gcnew Position (1,-1)},
		{gcnew Position (-1,-1),gcnew Position (0,-1),gcnew Position (0,0),gcnew Position (1,0)},
		{gcnew Position (-1,0),gcnew Position (0,0),gcnew Position (1,0),gcnew Position (0,-1)},
		{gcnew Position (-0.5,-0.5),gcnew Position (-0.5,0.5),gcnew Position (0.5,-0.5),gcnew Position (0.5,0.5)}
	};

	// ブロックの相対座標 絶対座標を設定
	for (int i = 0; i < blk_count; i++) {
		rel_positions [i] = blocks [type - 1, i];
		rot_positions [i] = blocks [type - 1, i];
		abs_positions [i] = gcnew Position (0, 0);
		Rel2Abs (rot_positions [i], abs_positions [i], %orig_position);
	}
}
void Block::Rel2Abs (Position^ rot_pos, Position^ abs_pos, Position^ orig) {
	abs_pos->x = orig->x + rot_pos->x;
	abs_pos->y = orig->y + rot_pos->y;
}
bool Block::Move (Position move_vec) {
	if (Game::controlable_flg) {
		Position new_orig (orig_position.x + move_vec.x, orig_position.y + move_vec.y);
		cli::array<Position^>^ rots = gcnew cli::array<Position^> (blk_count);
		cli::array<Position^>^ abses = gcnew cli::array<Position^> (blk_count);
		for (int i = 0; i < blk_count; i++) {
			rots [i] = rot_positions [i];
			abses [i] = gcnew Position ();
			Rel2Abs (rots [i], abses [i], %new_orig);
		}

		if (Game::Check_Field (abses, this)) {
			// 前いた場所からクリア
			for (int i = 0; i < blk_count; i++) {
				Game::squares [(int) abs_positions [i]->x, (int) abs_positions [i]->y]->Set_Block (nullptr);
			}

			// 新しい場所に移動
			for (int i = 0; i < blk_count; i++) {
				orig_position.x = new_orig.x;
				orig_position.y = new_orig.y;
				abs_positions [i]->x = abses [i]->x;
				abs_positions [i]->y = abses [i]->y;

				Game::squares [(int) abs_positions [i]->x, (int) abs_positions [i]->y]->Set_Block (this);
			}

			return true;
		} else {
			if (Game::Check_Accumulate (this, abs_positions)) {
				Game::Check_Fill ();
				Game::Wait (100);
				Game::Spawn_Block ();
			}
			return false;
		}
	} else {
		return false;
	}
}
bool Block::Rotate (int direction) {
	if (Game::controlable_flg) {
		cli::array<Position^>^ new_rot = gcnew cli::array<Position^> (blk_count);
		cli::array<Position^>^ new_abs = gcnew cli::array<Position^> (blk_count);

		// 回転行列を使ってブロックを90°ずつ回転
		int sin_c = (int) sin (PI / 2.0 * direction);
		int cos_c = (int) cos (PI / 2.0 * direction);
		for (int i = 0; i < blk_count; i++) {
			double x = rel_positions [i]->x;
			double y = rel_positions [i]->y;
			new_rot [i] = gcnew Position ((x * cos_c) - (y * sin_c), (x * sin_c) + (y * cos_c));
			new_abs [i] = gcnew Position (0, 0);
			Rel2Abs (new_rot [i], new_abs [i], %orig_position);
		}

		if (Game::Check_Field (new_abs, this)) {
			angle += direction;

			// 前いた場所をクリア
			for (int i = 0; i < blk_count; i++) {
				Game::squares [(int) abs_positions [i]->x, (int) abs_positions [i]->y]->Set_Block (nullptr);
			}

			// 新しい場所に移動
			for (int i = 0; i < blk_count; i++) {
				rot_positions [i]->x = new_rot [i]->x;
				rot_positions [i]->y = new_rot [i]->y;
				abs_positions [i]->x = new_abs [i]->x;
				abs_positions [i]->y = new_abs [i]->y;

				Game::squares [(int) abs_positions [i]->x, (int) abs_positions [i]->y]->Set_Block (this);
			}
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}
// ---------------------------------------------


// ---------- Class to manage squares ----------
void Square::Set_Block (Block^ set_block) {
	block = set_block;
	// ブロックの色設定
	cli::array<System::Drawing::Color>^ colors = gcnew cli::array<System::Drawing::Color>{
		System::Drawing::Color::FromArgb (180, 180, 180),
			System::Drawing::Color::FromArgb (0, 255, 255),
			System::Drawing::Color::FromArgb (0, 0, 255),
			System::Drawing::Color::FromArgb (255, 102, 0),
			System::Drawing::Color::FromArgb (0, 255, 0),
			System::Drawing::Color::FromArgb (255, 0, 0),
			System::Drawing::Color::FromArgb (128, 0, 128),
			System::Drawing::Color::FromArgb (255, 255, 0)
	};

	if (block == nullptr) {
		box->BackColor = colors [0];
	} else {
		box->BackColor = colors [block->type];
	}
}
// ----------------------------------------
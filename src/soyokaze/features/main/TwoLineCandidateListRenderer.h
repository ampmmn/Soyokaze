#pragma once

#include "StandardCandidateListRenderer.h"

class TwoLineCandidateListRenderer : public StandardCandidateListRenderer
{
public:
	/**
	  コマンド名と説明を二行表示するレンダラーを生成する
	*/
	TwoLineCandidateListRenderer();
	~TwoLineCandidateListRenderer() override;
};

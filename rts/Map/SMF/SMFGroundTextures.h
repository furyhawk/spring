/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef _SMF_GROUND_TEXTURES_H_
#define _SMF_GROUND_TEXTURES_H_

#include <vector>

#include "Map/BaseGroundTextures.h"
#include "Rendering/GL/PBO.h"

class CSMFMapFile;
class CSMFReadMap;

class CSMFGroundTextures: public CBaseGroundTextures
{
public:
	CSMFGroundTextures(CSMFReadMap* rm);
	~CSMFGroundTextures();

	void DrawUpdate() override;
	void DrawUpdateSquare(int texSquareX, int texSquareY) override;

	bool SetSquareLuaTexture(int texSquareX, int texSquareY, int texID) override;
	bool GetSquareLuaTexture(int texSquareX, int texSquareY, int texID, int texSizeX, int texSizeY, int texMipLevel) override;

	void BindSquareTextureArray() const override;
	void UnBindSquareTextureArray() const override;

	uint32_t GetSquareMipLevel(uint32_t i) const override { return squares[i].GetMipLevel(); }

protected:
	void LoadTiles(CSMFMapFile& file);
	void LoadSquareTextures(const int minLevel, const int maxLevel);
	void ConvolveHeightMap(const int mapWidth, const int mipLevel);
	bool RecompressTilesIfNeeded();
	void ExtractSquareTiles(const int texSquareX, const int texSquareY, const int mipLevel, GLint* tileBuf) const;
	void LoadSquareTexture(int x, int y, int level);

	inline bool TexSquareInView(int, int) const;

	CSMFReadMap* smfMap;

private:
	struct GroundSquare {
		enum {
			RAW_TEX_IDX = 0,
			LUA_TEX_IDX = 1,
		};

		GroundSquare(): textureIDs{0, 0}, texMipLevel(0), texDrawFrame(1) {}
		~GroundSquare();

		bool HasLuaTexture() const { return (textureIDs[LUA_TEX_IDX] != 0); }

		void SetRawTexture(uint32_t id) { textureIDs[RAW_TEX_IDX] = id; }
		void SetLuaTexture(uint32_t id) { textureIDs[LUA_TEX_IDX] = id; }
		void SetMipLevel(uint32_t l) { texMipLevel = l; }
		void SetDrawFrame(uint32_t f) { texDrawFrame = f; }

		uint32_t* GetTextureIDPtr() { return &textureIDs[RAW_TEX_IDX]; }
		uint32_t GetTextureID() const { return textureIDs[HasLuaTexture()]; }
		uint32_t GetMipLevel() const { return texMipLevel; }
		uint32_t GetDrawFrame() const { return texDrawFrame; }

	private:
		uint32_t textureIDs[2];
		uint32_t texMipLevel;
		uint32_t texDrawFrame;
	};

	// note: intentionally declared static (see ReadMap)
	static std::vector<GroundSquare> squares;

	static std::vector<int> tileMap;
	static std::vector<char> tiles;

	// FIXME? these are not updated at runtime
	static std::vector<float> heightMaxima;
	static std::vector<float> heightMinima;
	static std::vector<float> stretchFactors;

	// use Pixel Buffer Objects for async. uploading (DMA)
	PBO pbo;

	uint32_t tileArrayTex = 0;
	uint32_t tileTexFormat = 0;
};

#endif // _BF_GROUND_TEXTURES_H_

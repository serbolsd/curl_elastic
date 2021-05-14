#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include <map>
#include <vector>

struct cmpVector3i {
  bool operator()(const sf::Vector3i& lhs, const sf::Vector3i& rhs) const {
    return lhs.x < rhs.x
      || (lhs.x == rhs.x && (lhs.y < rhs.y
        || (lhs.y == rhs.y && lhs.z < rhs.z)));
  }
};

struct cmpVector3f {
  bool operator()(const sf::Vector3f& lhs, const sf::Vector3f& rhs) const {
    return lhs.x < rhs.x
      || (lhs.x == rhs.x && (lhs.y < rhs.y
        || (lhs.y == rhs.y && lhs.z < rhs.z)));
  }
};

class HeatMap
{
public:
		HeatMap(int width, int height, float decay = 0.15f, float momentum = 0.80f);
  ~HeatMap() { delete m_window; };

  void
  close() {
    m_window->close();
  };

  std::vector<std::vector<float>>
  getInfluenceMap() const {
    return m_influenceMap;
  };
 
  float
  getInfluence(int x, int y) {
    return m_influenceMap[x][y];
  };

  void
  setInfluence(int x, int y, float influence) {
    m_influenceMap[x][y] = influence;
  };

  void
  setInfluenceMap(sf::Vector2u mapSize);

  void
  propagation();

  void 
  propagateInfluence(int x, int y);

  void
  calculateInfluence(int x, int y, float& maxInf, float& minInf);

  void
  createHeatMap(std::vector<sf::Vector3i> data);

  void
  showHeatmap();

  sf::Color
  BlendColor(sf::Color color1, sf::Color color2, float fraction);

  void
  render();

 private:

  sf::Vector3f m_scale;

  int m_max;

  sf::Vector3i m_minPoint;

  sf::Vector3i m_maxPoint;

  std::map<sf::Vector3f, int, cmpVector3f> m_normalizedMap;

  std::vector<std::vector<float>> m_influenceMap;

  float m_decay;

  float m_momentum;

  sf::Vector2i m_size;

  sf::Vector2u m_mapSize;

  sf::RenderWindow* m_window;

  sf::Image m_heatmapImg;

  std::vector<sf::Vector3i> m_points;
};
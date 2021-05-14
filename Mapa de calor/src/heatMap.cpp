#include "..\include\heatMap.h"

float 
lerp(float a, float b, float f) {
  return (a * (1.0f - f)) + (b * f);
}

float
inverseLerp(float a, float b, float v) { 
  return (v - a) / (b - a); 
}

template<typename T>
static T
clamp(T number, T bot, T top) {
  return std::min(std::max(number, bot), top);
}

HeatMap::HeatMap(int width, int height, float decay, float momentum) {
  m_size.x = width;
  m_size.y = height;

  m_decay = decay;

  m_momentum = momentum;

  m_normalizedMap.clear();

  m_scale = { 0, 0, 0 };

  m_minPoint = { 0, 0, 0 };
  m_maxPoint = { 0, 0, 0 };

  m_window = new sf::RenderWindow(sf::VideoMode(m_size.x, m_size.y), "HeatMap");
}

void 
HeatMap::setInfluenceMap(sf::Vector2u mapSize) {
  //to create influence map with a size
  m_influenceMap.resize(mapSize.x);
  for (int i = 0; i < m_influenceMap.size(); ++i) {
    m_influenceMap[i].resize(mapSize.y);
  }

  sf::Vector2i pixelPos;
  for (auto pos : m_normalizedMap) {
    pixelPos.x = pos.first.x * (mapSize.x);
    pixelPos.y = pos.first.y * (mapSize.y);
    if ((pixelPos.x >= 0) && (pixelPos.x < mapSize.x) &&
      (pixelPos.y >= 0) && (pixelPos.y < mapSize.y)) {
      m_influenceMap[pixelPos.x][pixelPos.y] = pos.second;
    }
  }
}

void 
HeatMap::propagation() {
  for (int i = 0; i < m_mapSize.x; ++i) {
    for (int j = 0; j < m_mapSize.y; ++j) {
      propagateInfluence(i, j);
    }
  }
}

void HeatMap::propagateInfluence(int x, int y) {
  float maxInf = 0.0f;
  float minInf = 0.0f;

  //Up
  calculateInfluence(x, y - 1, maxInf, minInf);
  //Left
  calculateInfluence(x - 1, y, maxInf, minInf);
  //Right
  calculateInfluence(x + 1, y, maxInf, minInf);
  //Down
  calculateInfluence(x, y + 1, maxInf, minInf);
  //UP Left
  calculateInfluence(x - 1, y - 1, maxInf, minInf);
  //Up Right
  calculateInfluence(x + 1, y - 1, maxInf, minInf);
  //Down right
  calculateInfluence(x + 1, y + 1, maxInf, minInf);
  //Down Left
  calculateInfluence(x - 1, y + 1, maxInf, minInf);

  float newInfluence = minInf + maxInf;
  setInfluence(x, y, lerp(getInfluence(x, y), newInfluence, m_momentum));
}

void HeatMap::calculateInfluence(int x, int y, float& maxInf, float& minInf) {
  if ((x >= 0) && (x < m_mapSize.x) && (y >= 0) && (y < m_mapSize.y)) {
    float inf = getInfluence(x, y) * expf(-1 * m_decay);
    maxInf = std::max(inf, maxInf);
    minInf = std::min(inf, minInf);
  }
}

void
HeatMap::createHeatMap(std::vector<sf::Vector3i> data) {
  m_max = 0;
  m_minPoint = { 0,0,0 };
  m_maxPoint = { 0,0,0 };
  m_normalizedMap.clear();
  m_points = data;

  std::map<sf::Vector3i, int, cmpVector3i> countingMap;
  std::map<sf::Vector3f, int, cmpVector3f> normalizedMap;

  // To store the min and max point on the heatmap
  sf::Vector3i minPoint(0, 0, 0);
  sf::Vector3i maxPoint(0, 0, 0);
  sf::Vector3f scale(0.0f, 0.0f, 0.0f);

  int maxCount = 0;

  for (sf::Vector3i pos : data) {
    if (countingMap.find(pos) == countingMap.end()) {
      //save the position if don't save yet
      countingMap.insert(std::make_pair(pos, 1));
    }
    else { ++countingMap[pos]; }
    //Comparate the current point with the actual max point
    maxPoint.x = maxPoint.x > pos.x ? maxPoint.x : pos.x;
    maxPoint.y = maxPoint.y > pos.y ? maxPoint.y : pos.y;
    maxPoint.z = maxPoint.z > pos.z ? maxPoint.z : pos.z;

    //Comparate the current point with the actual min point
    minPoint.x = minPoint.x < pos.x ? minPoint.x : pos.x;
    minPoint.y = minPoint.y < pos.y ? minPoint.y : pos.y;
    minPoint.z = minPoint.z < pos.z ? minPoint.z : pos.z;
  }
  //Calculate the scale
  scale.x = static_cast<int>(std::abs(minPoint.x - maxPoint.x));
  scale.y = static_cast<int>(std::abs(minPoint.y - maxPoint.y));
  scale.z = static_cast<int>(std::abs(minPoint.z - maxPoint.z));

  for (auto pos : countingMap) {
    sf::Vector3f normalized(pos.first.x, pos.first.y, pos.first.z);

    normalized.x = (normalized.x - minPoint.x) / scale.x;
    normalized.y = (normalized.y - minPoint.y) / scale.y;
    normalized.z = (normalized.z - minPoint.z) / scale.z;

    normalizedMap.insert(std::make_pair(normalized, pos.second));

    maxCount = maxCount > pos.second ? maxCount : pos.second;
  }

  m_max = maxCount;
  m_minPoint = minPoint;
  m_maxPoint = maxPoint;
  m_scale = scale;

  m_normalizedMap = normalizedMap;


}

void
HeatMap::showHeatmap() {
  int width = 1280;
  int height = 720;

  if (m_scale.x > m_scale.y) {
    m_mapSize.x = width;
    m_mapSize.y = width * m_scale.y / m_scale.x;
  }
  else {
    m_mapSize.x = height * m_scale.x / m_scale.y;
    m_mapSize.y = height;
  }

  sf::View view = m_window->getView();

  view.setSize(m_mapSize.x, m_mapSize.y);
  m_window->setView(view);
  setInfluenceMap(m_mapSize);
  propagation();

  sf::Image heatmapImg;
  heatmapImg.create(m_mapSize.x, m_mapSize.y, sf::Color::Transparent);

  sf::Vector2i pixelPosition;
  sf::Color c;

  sf::Color hottestC;
  hottestC.r = 255;
  hottestC.g = 71;
  hottestC.b = 71;
  hottestC.a = 0.8f * 255;
  sf::Color hotC;
  hotC.r = 255;
  hotC.g = 0;
  hotC.b = 179;
  hotC.a = 0.6f * 255;
  sf::Color coldC;
  coldC.r = 60;
  coldC.g = 88;
  coldC.b = 191;
  coldC.a = 0.4 * 255;
  sf::Color coldestC;
  coldestC.r = 121;
  coldestC.g = 255;
  coldestC.b = 223;
  coldestC.a = 0.3 * 255;
  sf::Color tranC = sf::Color::Transparent;

  float maxValue = 0;

  for (auto map : getInfluenceMap()) {
    for (auto pos : map) {
      maxValue = std::max(maxValue, pos);
    }
  }

  int posX = 0;
  int posY = 0;

  for (auto map : getInfluenceMap()) {
    posY = 0;
    for (auto pos : map) {
      float normPos = pos / maxValue;
      // Defines the color that should appear at a given point
      c = pos > 0.75 ? BlendColor(hotC, hottestC, clamp(inverseLerp(0.75, 1.5, pos), 0.0f, 1.0f)) :
          pos > 0.50 ? BlendColor(coldC, hotC, inverseLerp(0.50, 0.75, pos)) :
          pos > 0.25 ? BlendColor(coldestC, coldC, inverseLerp(0.25, 0.50, pos)) :
          BlendColor(tranC, coldestC, inverseLerp(0.00, 0.25, pos));

      heatmapImg.setPixel(posX, posY, c);
      ++posY;
    }
    ++posX;
  }

  sf::Texture heatMapTexture;
  heatMapTexture.create(m_mapSize.x, m_mapSize.y);
  heatMapTexture.update(heatmapImg);
  sf::Sprite heatMapSprite;
  heatMapSprite.setTexture(heatMapTexture);

  m_window->clear();
  m_window->draw(heatMapSprite);
  m_window->display();
}

sf::Color 
HeatMap::BlendColor(sf::Color color1, sf::Color color2, float fraction) {
  sf::Color oColor;

  oColor.r = static_cast<sf::Uint8>((color2.r - color1.r) * fraction + color1.r);
  oColor.g = static_cast<sf::Uint8>((color2.g - color1.g) * fraction + color1.g);
  oColor.b = static_cast<sf::Uint8>((color2.b - color1.b) * fraction + color1.b);
  oColor.a = static_cast<sf::Uint8>((color2.a - color1.a) * fraction + color1.a);

  return oColor;
}

void HeatMap::render() {
  sf::Clock deltaClock;
  float time=0;
  while (m_window->isOpen()) {
    sf::Event event;
    while (m_window->pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        m_window->close();
    }
    if (time >=1) {
      showHeatmap();
      //propagation();
      //for (int i = 0; i < m_points.size(); i++)
      //{
      //  //setInfluence(m_points[i].x, m_points[i].y, 1);
      //}
      time = 0;
    }
    time += deltaClock.restart().asSeconds();
  }
}

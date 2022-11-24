// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "JumperInsterter.hpp"

#include <iostream>
#include <algorithm>

JumperInserter::JumperInserter() : foundNetsNumber(0), time_start(0), time_stop(0), violationsFixed(0), violationsLeaved(0) {}

bool JumperInserter::ProcessViolatedNets(DEF &def, AntennaViolations &violations, bool doEcho)  {
  time_start = std::clock();
  std::cout << "====== Searching for violated nets in DEF file ======" << std::endl;
  violationsLeaved  = violations.size();
  violationsFixed   = 0;
  for (size_t i = 0; i < violations.size(); ++i)
    ProcessViolation(def, violations[i], doEcho);
  std::cout << foundNetsNumber / violations.size() * 100 << "% of violated nets found" << std::endl;
  time_stop = std::clock();
  return true;
}

void JumperInserter::PrintStatistics() {
  std::cout << "Number of fixed violations : " << violationsFixed  << " (of " << violationsFixed + violationsLeaved << " violations total)" << std::endl;
  std::cout << "Number of leaved violations: " << violationsLeaved << " (of " << violationsFixed + violationsLeaved << " violations total)" << std::endl;
  std::cout << "Antenna voilations fixed in " << (time_stop - time_start) << " msec" << std::endl << std::endl;
}

bool JumperInserter::ProcessViolation(DEF& def, AntennaViolation& violation, bool doEcho) {
  std::vector<size_t>   indices;
  std::vector<int32_t>  values;
  size_t                metalIndex = 0;

  for (size_t j = 0; j < def.nets.size(); ++j) {
    if (violation.net.netName != def.nets[j].name)
      continue;
    ++foundNetsNumber;
    if (doEcho) {
      std::cout << "  * net '" << violation.net.netName << "' found in DEF file [ position " << j << " ]" << std::endl;
      std::cout << "    Area: '" << violation.value << "' is bigger then allowd value of '" << violation.maxAllowedValue << "' [ x " << (violation.value / violation.maxAllowedValue) << " ]" << std::endl;
    }
    int numberOfSegments = static_cast<int>(ceil(violation.value / violation.maxAllowedValue));
    if (doEcho)
      std::cout << "    Will create " << numberOfSegments << " segments (" << numberOfSegments - 1 << " cut(s) to be done)" << std::endl;

    indices.clear();
    values.clear();
    for (size_t k = 0; k < def.nets[j].routingPoints.size(); ++k)
      if (def.nets[j].routingPoints[k].name == violation.net.layerName && def.nets[j].routingPoints[k].mode == LastPointMode::point) {
        indices.push_back(k);
        Point delta = { def.nets[j].routingPoints[k].last.leftBottom.x - def.nets[j].routingPoints[k].first.x, def.nets[j].routingPoints[k].last.leftBottom.y - def.nets[j].routingPoints[k].first.y };
        if (!delta.y) {
          if (doEcho)
            std::cout << "      record [" << k << "] for metal " << def.nets[j].routingPoints[k].name << " of type POINT found, deltaX=" << delta.x << std::endl;
          values.push_back(delta.x);
        }
        else {
          if (doEcho)
            std::cout << "      record [" << k << "] for metal " << def.nets[j].routingPoints[k].name << " of type POINT found, deltaY=" << delta.y << std::endl;
          values.push_back(delta.y);
        }
        metalIndex = violation.net.layerName[violation.net.layerName.length() - 1] - '0';
      }
    if (indices.size() > 1)
      for (size_t a = 0; a < values.size(); ++a)
        for (size_t b = 1; b < values.size(); ++b)
          if (values[b] > values[b - 1]) {
            int32_t temp1 = values[b];
            values[b] = values[b - 1];
            values[b - 1] = temp1;

            size_t temp2 = indices[b];
            indices[b] = indices[b - 1];
            indices[b - 1] = temp2;
          }
    if (doEcho)
      std::cout << "    > max delta (" << values[0] << ") found for index " << indices[0] << std::endl;

    Point delta = { def.nets[j].routingPoints[indices[0]].last.leftBottom.x - def.nets[j].routingPoints[indices[0]].first.x, def.nets[j].routingPoints[indices[0]].last.leftBottom.y - def.nets[j].routingPoints[indices[0]].first.y };
    std::string metalLayerName = "met0";
    metalLayerName[metalLayerName.length() - 1] = '0' + (char)metalIndex + 1;
    std::vector<int32_t> coords;

    if (violation.value / violation.maxAllowedValue > 2) {
      std::cerr << "__err__: Only nets with excess width less than x2 are now supported (but x" << violation.value / violation.maxAllowedValue << "was found)" << std::endl;
      std::cerr << "         Violation will be skipped :(" << std::endl;
      continue;
    }

    if (delta.y == 0) {
      // We are HORIZONTAL line
      if (doEcho)
        std::cout << "    Analyzing metallization intersections for " << violation.net.netName << " (" << def.nets[j].routingPoints[indices[0]].first.x << ", " << def.nets[j].routingPoints[indices[0]].first.y << ") -> (" << def.nets[j].routingPoints[indices[0]].last.leftBottom.x << ", *)" << std::endl;
      for (size_t a = 0; a < def.nets.size(); ++a)
        for (size_t b = 0; b < def.nets[a].routingPoints.size(); ++b) {
          // Пропускаем металлы, которые не на нашем слое
          if (def.nets[a].routingPoints[b].name != metalLayerName)
            continue;
          // Пропускаем металлы, у которых конечным объектом является не координата
          if (def.nets[a].routingPoints[b].mode != LastPointMode::point)
            continue;
          // Пропускаем металлы, у которых координаты по x выходят за пределы нашего сегмента
          if (def.nets[a].routingPoints[b].first.x < def.nets[j].routingPoints[indices[0]].first.x || def.nets[a].routingPoints[b].first.x > def.nets[j].routingPoints[indices[0]].last.leftBottom.x)
            continue;
          // Пропускаем металлы, у которых координаты по y выходят за пределы нашего сегмента
          if (def.nets[a].routingPoints[b].first.y > def.nets[j].routingPoints[indices[0]].first.y || def.nets[a].routingPoints[b].last.leftBottom.y < def.nets[j].routingPoints[indices[0]].first.y)
            continue;
          // Пропускаем металлы, у которых конечным объектом является не координата
          if (def.nets[a].routingPoints[b].mode != LastPointMode::point) {
            std::cerr << "__wrn__: Routing points with last point mode != POINT are not supported" << std::endl;
            continue;
          }
          coords.push_back(def.nets[a].routingPoints[b].first.x);
          if (doEcho)
            std::cout << "      - Intersecting segment at layer " << metalLayerName << ": (" << def.nets[a].routingPoints[b].first.x << ", " << def.nets[a].routingPoints[b].first.y << ") -> (" << def.nets[a].routingPoints[b].last.leftBottom.x << ", " << def.nets[a].routingPoints[b].last.leftBottom.y << ") " << std::endl;
        }
      // Начинаем строить сечения
      if (coords.empty()) {
        // Пока мы можем работать только на металле 1
        if (def.nets[j].routingPoints[indices[0]].name != "met1") {
          std::cerr << "__wrn__: Can process only violations at layer METAL1" << std::endl;
          std::cerr << "         By now will ignore this violation" << std::endl;
          continue;
        }
        if (doEcho)
          std::cout << "      ! No intersecting segments found" << std::endl;
        Point segmentStartPoint = def.nets[j].routingPoints[indices[0]].first;
        Point segmentEndPoint = def.nets[j].routingPoints[indices[0]].last.leftBottom;
        int length = segmentEndPoint.x - segmentStartPoint.x;

        Point segmentMiddlePointA = { segmentStartPoint.x + (length - 340) / 2, segmentStartPoint.y };
        Point segmentMiddlePointB = { segmentMiddlePointA.x + 340, segmentMiddlePointA.y };

        RoutedMetal metal = def.nets[j].routingPoints[indices[0]];

        // Идём слева направо, обрываем металл посередине
        def.nets[j].routingPoints[indices[0]].last.leftBottom = segmentMiddlePointA;

        // В этом же месте вставляем VIA на следующий металл
        RoutedMetal metalVIAUp;
        metalVIAUp.name = def.nets[j].routingPoints[indices[0]].name;
        metalVIAUp.viaName = "M1M2_PR";
        metalVIAUp.first = segmentMiddlePointA;
        metalVIAUp.last.leftBottom = { 0, 0 };
        metalVIAUp.last.rightTop = { 0, 0 };
        metalVIAUp.mode = LastPointMode::via;
        def.nets[j].routingPoints.insert(def.nets[j].routingPoints.begin() + indices[0] + 1, metalVIAUp);

        // Из этой точки ведём второй металл размером в 1 трек
        RoutedMetal metalUp = metalVIAUp;
        metalUp.viaName.erase();
        metalUp.name = "met2";
        metalUp.last.leftBottom = segmentMiddlePointB;
        metalUp.mode = LastPointMode::point;
        def.nets[j].routingPoints.insert(def.nets[j].routingPoints.begin() + indices[0] + 2, metalUp);

        // Теперь тут вставляем VIA на предыдущий металл
        RoutedMetal metalVIADown;
        metalVIADown.name = def.nets[j].routingPoints[indices[0]].name;
        metalVIADown.viaName = "M1M2_PR";
        metalVIADown.first = segmentMiddlePointB;
        metalVIADown.last.leftBottom = { 0, 0 };
        metalVIADown.last.rightTop = { 0, 0 };
        metalVIADown.mode = LastPointMode::via;
        def.nets[j].routingPoints.insert(def.nets[j].routingPoints.begin() + indices[0] + 3, metalVIADown);

        // Идём дальше, вставляем сразу после этого новый металл на оставшуюся длину
        metal.first = segmentMiddlePointB;
        def.nets[j].routingPoints.insert(def.nets[j].routingPoints.begin() + indices[0] + 4, metal);
      }
      else {
        // Пока мы можем работать только на металле 1
        if (def.nets[j].routingPoints[indices[0]].name != "met1") {
          std::cerr << "__wrn__: Can process only violations at layer METAL1" << std::endl;
          std::cerr << "         By now will ignore this violation" << std::endl;
          continue;
        }
        if (doEcho)
          std::cout << "      ! " << coords.size() << " intersecting segments found" << std::endl;

        Point segmentStartPoint = def.nets[j].routingPoints[indices[0]].first;
        Point segmentEndPoint = def.nets[j].routingPoints[indices[0]].last.leftBottom;
        int length = segmentEndPoint.x - segmentStartPoint.x;

        // Задача №1. Необходимо найти участок около центра, в который мы можем воткнуть джампер
        Point segmentMiddlePoint = { segmentStartPoint.x + (length - 340) / 2, segmentStartPoint.y };
        size_t  prevPointIndex = 0, 
                nextPointIndex = 0;

        // TODO: Отдельно нужно робрабатывать ситуацию, если у нас всего одна точка пересечения!
        // TODO: Нужно посмотреть, а что, если мы перед первой точкой или послел последней?
        // Ищем, между какими точками у нас получается середина
        std::sort(coords.begin(), coords.end());
        for (size_t i = 0; i < coords.size(); ++i) {
          if (coords[i] <= segmentMiddlePoint.x)
            continue;
          prevPointIndex = i - 1;
          nextPointIndex = i;
          break;
        }

        // Вычисляем расстояние между точками с целью узнать, можем ли мы сюда воткнуться
        int distance = 0;
        // Если нам нужно воткнуться перед первой точкой пересечения металлов
        if (nextPointIndex == 0)
          distance = coords[nextPointIndex] - segmentStartPoint.x;
        else
          // Если нам нужно воткнуться после последней точки пересечения металлов
          if (prevPointIndex == coords.size() - 1)
            distance = segmentEndPoint.x - coords[prevPointIndex];
          else
            distance = coords[nextPointIndex] - coords[prevPointIndex];

        //TODO: И тут снова магисеское число! Заменить на фактическую ширину трека!
        if (distance > 3 * 340) {

          Point segmentMiddlePointA = { segmentStartPoint.x + (length - 340) / 2, segmentStartPoint.y };
          Point segmentMiddlePointB = { segmentMiddlePointA.x + 340, segmentMiddlePointA.y };

          // Двигаем участки влево/вправо, что уместить внутри диапазона, если мы попадаем на границу пересечения металлов
          
          if (nextPointIndex != 0 && segmentMiddlePointA.x - 340 < coords[prevPointIndex]) {
            int delta = coords[prevPointIndex] - (segmentMiddlePointA.x - 340);
            segmentMiddlePointA.x += delta;
            segmentMiddlePointB.x += delta;
          } else 
            if (prevPointIndex != coords.size() - 1 && segmentMiddlePointB.x + 340 > coords[nextPointIndex]) {
              int delta = (segmentMiddlePointB.x + 340) - coords[nextPointIndex];
              segmentMiddlePointA.x -= delta;
              segmentMiddlePointB.x -= delta;
            }

          RoutedMetal metal = def.nets[j].routingPoints[indices[0]];

          // Идём слева направо, обрываем металл посередине
          def.nets[j].routingPoints[indices[0]].last.leftBottom = segmentMiddlePointA;

          // В этом же месте вставляем VIA на следующий металл
          RoutedMetal metalVIAUp;
          metalVIAUp.name = def.nets[j].routingPoints[indices[0]].name;
          metalVIAUp.viaName = "M1M2_PR";
          metalVIAUp.first = segmentMiddlePointA;
          metalVIAUp.last.leftBottom = { 0, 0 };
          metalVIAUp.last.rightTop = { 0, 0 };
          metalVIAUp.mode = LastPointMode::via;
          def.nets[j].routingPoints.insert(def.nets[j].routingPoints.begin() + indices[0] + 1, metalVIAUp);

          // Из этой точки ведём второй металл размером в 1 трек
          RoutedMetal metalUp = metalVIAUp;
          metalUp.viaName.erase();
          metalUp.name = "met2";
          metalUp.last.leftBottom = segmentMiddlePointB;
          metalUp.mode = LastPointMode::point;
          def.nets[j].routingPoints.insert(def.nets[j].routingPoints.begin() + indices[0] + 2, metalUp);

          // Теперь тут вставляем VIA на предыдущий металл
          RoutedMetal metalVIADown;
          metalVIADown.name = def.nets[j].routingPoints[indices[0]].name;
          metalVIADown.viaName = "M1M2_PR";
          metalVIADown.first = segmentMiddlePointB;
          metalVIADown.last.leftBottom = { 0, 0 };
          metalVIADown.last.rightTop = { 0, 0 };
          metalVIADown.mode = LastPointMode::via;
          def.nets[j].routingPoints.insert(def.nets[j].routingPoints.begin() + indices[0] + 3, metalVIADown);

          // Идём дальше, вставляем сразу после этого новый металл на оставшуюся длину
          metal.first = segmentMiddlePointB;
          def.nets[j].routingPoints.insert(def.nets[j].routingPoints.begin() + indices[0] + 4, metal);
        }
        else {
          // TODO: Что делать, если место, куда нам нужно воткнуться, слишком "узкое"?
          std::cerr << "> Can't process points! :(" << std::endl;
        }

        Point segmentMiddlePointA = { segmentStartPoint.x + (length - 340) / 2, segmentStartPoint.y };
        Point segmentMiddlePointB = { segmentMiddlePointA.x + 340, segmentMiddlePointA.y };

        RoutedMetal metal = def.nets[j].routingPoints[indices[0]];
  
      }

    } // End Of : We are HORIZONTAL line
    else {
      // We are VERTICAL line
    } // We are VERTICAL line
  }
  return true;
}

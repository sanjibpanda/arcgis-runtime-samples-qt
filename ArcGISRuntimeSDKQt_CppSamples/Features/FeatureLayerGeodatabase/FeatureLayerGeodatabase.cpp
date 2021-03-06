// [WriteFile Name=FeatureLayerGeodatabase, Category=Features]
// [Legal]
// Copyright 2016 Esri.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// [Legal]

#include "FeatureLayerGeodatabase.h"

#include "Map.h"
#include "MapQuickView.h"
#include "FeatureLayer.h"
#include "Basemap.h"
#include "GeodatabaseFeatureTable.h"
#include "Geodatabase.h"
#include "ArcGISVectorTiledLayer.h"
#include "SpatialReference.h"
#include "Point.h"
#include "Viewpoint.h"
#include <QQmlProperty>
#include <QUrl>

using namespace Esri::ArcGISRuntime;

FeatureLayerGeodatabase::FeatureLayerGeodatabase(QQuickItem* parent) :
    QQuickItem(parent),
    m_map(nullptr),
    m_mapView(nullptr),
    m_geodatabase(nullptr),
    m_dataPath("")
{
}

FeatureLayerGeodatabase::~FeatureLayerGeodatabase()
{
}

void FeatureLayerGeodatabase::componentComplete()
{
    QQuickItem::componentComplete();

    // find QML MapView component
    m_mapView = findChild<MapQuickView*>("mapView");
    m_mapView->setWrapAroundMode(WrapAroundMode::Disabled);

    // get the data path
    m_dataPath = QQmlProperty::read(m_mapView, "dataPath").toString();

    // Create a map using a local vector tile package
    auto vectorTiledLayer = new ArcGISVectorTiledLayer(QUrl::fromLocalFile(m_dataPath + "vtpk/LosAngeles.vtpk"), this);
    auto basemap = new Basemap(vectorTiledLayer, this);
    m_map = new Map(basemap, this);

    // Set map to map view
    m_mapView->setMap(m_map);

    // set initial viewpoint
    m_map->setInitialViewpoint(Viewpoint(Point(-13214155, 4040194, SpatialReference(3857)), 35e4));

    // create the geodatabase using a local file path
    m_geodatabase = new Geodatabase(m_dataPath + "geodatabase/LA_Trails.geodatabase", this);

    // connect to doneLoading signal of geodatabase to access feature tables
    connect(m_geodatabase, &Geodatabase::doneLoading, [this](Error error)
    {
        if (error.isEmpty())
        {
            // access the feature table by name
            auto featureTable = m_geodatabase->geodatabaseFeatureTable("Trailheads");

            // create a feature layer from the feature table
            auto featureLayer = new FeatureLayer(featureTable, this);

            // add the feature layer to the map
            m_map->operationalLayers()->append(featureLayer);
        }
    });

    // load the geodatabase
    m_geodatabase->load();
}

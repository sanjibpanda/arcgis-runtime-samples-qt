// [WriteFile Name=AddFeaturesFeatureService, Category=EditData]
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

#include "AddFeaturesFeatureService.h"

#include "Map.h"
#include "MapQuickView.h"
#include "Basemap.h"
#include "Viewpoint.h"
#include "Point.h"
#include "SpatialReference.h"
#include "ServiceFeatureTable.h"
#include "FeatureLayer.h"
#include "Feature.h"
#include "FeatureEditResult.h"
#include <QUrl>
#include <QMap>
#include <QUuid>
#include <QSharedPointer>
#include <QVariant>
#include <QMouseEvent>

using namespace Esri::ArcGISRuntime;

AddFeaturesFeatureService::AddFeaturesFeatureService(QQuickItem* parent) :
    QQuickItem(parent),
    m_map(nullptr),
    m_mapView(nullptr),
    m_featureLayer(nullptr),
    m_featureTable(nullptr)
{
}

AddFeaturesFeatureService::~AddFeaturesFeatureService()
{
}

void AddFeaturesFeatureService::componentComplete()
{
    QQuickItem::componentComplete();

    // find QML MapView component
    m_mapView = findChild<MapQuickView*>("mapView");
    m_mapView->setWrapAroundMode(WrapAroundMode::Disabled);

    // create a Map by passing in the Basemap
    m_map = new Map(Basemap::streets(this), this);
    m_map->setInitialViewpoint(Viewpoint(Point(-10800000, 4500000, SpatialReference(102100)), 3e7));

    // set map on the map view
    m_mapView->setMap(m_map);

    // create the ServiceFeatureTable
    m_featureTable = new ServiceFeatureTable(QUrl("http://sampleserver6.arcgisonline.com/arcgis/rest/services/DamageAssessment/FeatureServer/0"), this);

    // create the FeatureLayer with the ServiceFeatureTable and add it to the Map
    m_featureLayer = new FeatureLayer(m_featureTable, this);
    m_featureLayer->setSelectionWidth(3);
    m_map->operationalLayers()->append(m_featureLayer);

    connectSignals();
}

void AddFeaturesFeatureService::connectSignals()
{
    // connect to the mouse press release signal on the MapQuickView
    connect(m_mapView, &MapQuickView::mouseClick, [this](QMouseEvent& mouseEvent)
    {
        // obtain the map point
        Point newPoint = m_mapView->screenToLocation(mouseEvent.x(), mouseEvent.y());

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;
        featureAttributes.insert("typdamage", "Minor");
        featureAttributes.insert("primcause", "Earthquake");

        // create a new feature and add it to the feature table
        Feature* feature = m_featureTable->createFeature(featureAttributes,newPoint,this);
        m_featureTable->addFeature(feature);
    });

    // connect to the addFeatureCompleted signal from the ServiceFeatureTable
    connect(m_featureTable, &ServiceFeatureTable::addFeatureCompleted, [this](QUuid, bool success)
    {
        // if add feature was successful, call apply edits
        if (success)
            m_featureTable->applyEdits();
    });

    // connect to the applyEditsCompleted signal from the ServiceFeatureTable
    connect(m_featureTable, &ServiceFeatureTable::applyEditsCompleted, [this](QUuid, QList<QSharedPointer<FeatureEditResult>> featureEditResults)
    {
        if (featureEditResults.isEmpty())
            return;
        // obtain the first item in the list
        QSharedPointer<FeatureEditResult> featureEditResult = featureEditResults.first();
        // check if there were errors, and if not, log the new object ID
        if (!featureEditResult->isCompletedWithErrors())
            qDebug() << "New Object ID is:" << featureEditResult->objectId();
        else
            qDebug() << "Apply edits error.";
    });
}

#include "georeferenced.h"

#include <QtMath>
#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include <iostream>

Georeferenced::Georeferenced(): geoTransform{0.0,1.0,0.0,0.0,0.0,1.0}, inverseGeoTransform{0.0,1.0,0.0,0.0,0.0,1.0}, projectTransformation(0), unprojectTransformation(0)
{

}

QPointF Georeferenced::pixelToProjectedPoint(const QPointF &point) const
{
    return QPointF(geoTransform[0]+point.x()*geoTransform[1]+point.y()*geoTransform[2],
                   geoTransform[3]+point.x()*geoTransform[4]+point.y()*geoTransform[5]);
}

QPointF Georeferenced::projectedPointToPixel(const QPointF &point) const
{
    return QPointF(inverseGeoTransform[0]+point.x()*inverseGeoTransform[1]+point.y()*inverseGeoTransform[2],
                   inverseGeoTransform[3]+point.x()*inverseGeoTransform[4]+point.y()*inverseGeoTransform[5]);
}

void Georeferenced::extractGeoreference(GDALDataset *dataset)
{
    dataset->GetGeoTransform(geoTransform);
    GDALInvGeoTransform(geoTransform,inverseGeoTransform);

    OGRSpatialReference projected, wgs84;

    std::cerr << dataset->GetProjectionRef() << std::endl;

    char * wktProjection = const_cast<char *>(dataset->GetProjectionRef());
    projected.importFromWkt(&wktProjection);

    wgs84.SetWellKnownGeogCS("WGS84");

    unprojectTransformation = OGRCreateCoordinateTransformation(&projected,&wgs84);
    projectTransformation = OGRCreateCoordinateTransformation(&wgs84,&projected);
}

QPointF Georeferenced::project(const QGeoCoordinate &point) const
{
    if(projectTransformation)
    {
        double x = point.longitude();
        double y = point.latitude();
        projectTransformation->Transform(1,&x,&y);
        return QPointF(x,y);
    }
    return QPointF();
}

QGeoCoordinate Georeferenced::unproject(const QPointF &point) const
{
    if(unprojectTransformation)
    {
        double x = point.x();
        double y = point.y();
        unprojectTransformation->Transform(1,&x,&y);
        return QGeoCoordinate(y, x);
    }
    return QGeoCoordinate();
}


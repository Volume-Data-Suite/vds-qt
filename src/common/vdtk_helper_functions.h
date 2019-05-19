#pragma once

#include <VDTK/common/CommonDataTypes.h>
#include <QVector3D>

namespace VDS::Helper
{
inline const VDTK::VolumeSize QVector3DToVolumeSize(const QVector3D size) {
	return VDTK::VolumeSize(static_cast<uint32_t>(size.x()), static_cast<uint32_t>(size.y()), static_cast<uint32_t>(size.z()));
}

inline const VDTK::VolumeSpacing QVector3DToVolumeSpacing(const QVector3D spacing) {
	return VDTK::VolumeSpacing(spacing.x(), spacing.y(), spacing.z());
}

inline const QVector3D VolumeSizetoQVector3D(const VDTK::VolumeSize size) {
	return QVector3D(static_cast<float>(size.getX()), static_cast<float>(size.getY()), static_cast<float>(size.getZ()));
}

inline const QVector3D VolumeSpacingToQVector3D(const VDTK::VolumeSpacing spacing) {
	return QVector3D(spacing.getX(), spacing.getY(), spacing.getZ());
}
}


/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include "pvguiqt/PVLogoModel.h"

#include <QFile>
#include <QTextStream>
#include <QVarLengthArray>

#include <QtOpenGL>

PVGuiQt::PVLogoModel::PVLogoModel(const QString& filePath)
    : m_fileName(QFileInfo(filePath).fileName())
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
		return;

	PVPoint3D boundsMin(1e9, 1e9, 1e9);
	PVPoint3D boundsMax(-1e9, -1e9, -1e9);

	QTextStream in(&file);
	while (!in.atEnd()) {
		QString input = in.readLine();
		if (input.isEmpty() || input[0] == '#')
			continue;

		QTextStream ts(&input);
		QString id;
		ts >> id;
		if (id == "v") {
			PVPoint3D p;
			for (int i = 0; i < 3; ++i) {
				ts >> p[i];
				boundsMin[i] = qMin(boundsMin[i], p[i]);
				boundsMax[i] = qMax(boundsMax[i], p[i]);
			}
			m_points << p;
		} else if (id == "f" || id == "fo") {
			QVarLengthArray<int, 4> p;

			while (!ts.atEnd()) {
				QString vertex;
				ts >> vertex;
				const int vertexIndex = vertex.split('/').value(0).toInt();
				if (vertexIndex)
					p.append(vertexIndex > 0 ? vertexIndex - 1 : m_points.size() + vertexIndex);
			}

			for (int i = 0; i < p.size(); ++i) {
				const int edgeA = p[i];
				const int edgeB = p[(i + 1) % p.size()];

				if (edgeA < edgeB)
					m_edgeIndices << edgeA << edgeB;
			}

			for (int i = 0; i < 3; ++i)
				m_pointIndices << p[i];

			if (p.size() == 4)
				for (int i = 0; i < 3; ++i)
					m_pointIndices << p[(i + 2) % 4];
		}
	}

	const PVPoint3D bounds = boundsMax - boundsMin;
	const qreal scale = 1 / qMax(bounds.x, qMax(bounds.y, bounds.z));
	for (int i = 0; i < m_points.size(); ++i)
		m_points[i] = (m_points[i] - (boundsMin + bounds * 0.5)) * scale;

	m_normals.resize(m_points.size());
	for (int i = 0; i < m_pointIndices.size(); i += 3) {
		const PVPoint3D a = m_points.at(m_pointIndices.at(i));
		const PVPoint3D b = m_points.at(m_pointIndices.at(i + 1));
		const PVPoint3D c = m_points.at(m_pointIndices.at(i + 2));

		const PVPoint3D normal = cross(b - a, c - a).normalize();

		for (int j = 0; j < 3; ++j)
			m_normals[m_pointIndices.at(i + j)] += normal;
	}

	for (int i = 0; i < m_normals.size(); ++i)
		m_normals[i] = m_normals[i].normalize();
}

void PVGuiQt::PVLogoModel::render(bool wireframe, bool normals) const
{
	/*glRotated(-90, 1, 0, 1);
	glRotated(-45, 0, 1, 0);
	glScalef(1.0f, 0.3f, 1.0f);*/
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	if (wireframe) {
		glVertexPointer(3, GL_FLOAT, 0, (float*)m_points.data());
		glDrawElements(GL_LINES, m_edgeIndices.size(), GL_UNSIGNED_INT, m_edgeIndices.data());
	} else {
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glShadeModel(GL_SMOOTH);

		glEnableClientState(GL_NORMAL_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (float*)m_points.data());
		glNormalPointer(GL_FLOAT, 0, (float*)m_normals.data());
		glDrawElements(GL_TRIANGLES, m_pointIndices.size(), GL_UNSIGNED_INT, m_pointIndices.data());

		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);
	}

	if (normals) {
		QVector<PVPoint3D> normals;
		for (int i = 0; i < m_normals.size(); ++i)
			normals << m_points.at(i) << (m_points.at(i) + m_normals.at(i) * 0.02f);
		glVertexPointer(3, GL_FLOAT, 0, (float*)normals.data());
		glDrawArrays(GL_LINES, 0, normals.size());
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_DEPTH_TEST);
}

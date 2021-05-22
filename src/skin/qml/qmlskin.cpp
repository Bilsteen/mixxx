#include "skin/qml/qmlskin.h"

#include <QQmlEngine>
#include <QQuickWidget>

#include "util/assert.h"

namespace {
const QString kSkinMetadataFileName = QStringLiteral("skin.ini");
const QString kMainQmlFileName = QStringLiteral("main.qml");
} // namespace

namespace mixxx {
namespace skin {
namespace qml {

// static
SkinPointer QmlSkin::fromDirectory(const QDir& dir) {
    if (dir.exists(kSkinMetadataFileName) && dir.exists(kMainQmlFileName)) {
        return std::make_shared<QmlSkin>(QFileInfo(dir.absolutePath()));
    }
    return nullptr;
}

QmlSkin::QmlSkin(const QFileInfo& path)
        : m_path(path),
          m_settings(path.absoluteFilePath() + QStringLiteral("/") +
                          kSkinMetadataFileName,
                  QSettings::IniFormat) {
    DEBUG_ASSERT(isValid());
}

bool QmlSkin::isValid() const {
    return !m_path.filePath().isEmpty() && m_settings.status() == QSettings::NoError;
}

QFileInfo QmlSkin::path() const {
    DEBUG_ASSERT(isValid());
    return m_path;
}

QPixmap QmlSkin::preview(const QString& schemeName = QString()) const {
    Q_UNUSED(schemeName);
    DEBUG_ASSERT(schemeName.isEmpty());
    DEBUG_ASSERT(isValid());
    QPixmap preview;
    preview.load(m_path.absoluteFilePath() + QStringLiteral("/skin_preview.png"));
    if (preview.isNull()) {
        preview.load(":/images/skin_preview_placeholder.png");
    }
    return preview;
}

QString QmlSkin::name() const {
    DEBUG_ASSERT(isValid());
    return m_path.fileName();
}

QList<QString> QmlSkin::colorschemes() const {
    DEBUG_ASSERT(isValid());
    // TODO: Implement this
    return {};
}

QString QmlSkin::description() const {
    DEBUG_ASSERT(isValid());
    return m_settings.value("Skin/description", QString()).toString();
}

bool QmlSkin::fitsScreenSize(const QScreen& screen) const {
    DEBUG_ASSERT(isValid());
    const auto screenSize = screen.size();
    const int minScreenWidth = m_settings.value("Skin/min_pixel_width", -1).toInt();
    if (minScreenWidth >= 0 && minScreenWidth < screenSize.width()) {
        return false;
    }

    const int minScreenHeight = m_settings.value("Skin/min_pixel_height", -1).toInt();
    if (minScreenWidth >= 0 && minScreenHeight < screenSize.height()) {
        return false;
    }

    return true;
}

LaunchImage* QmlSkin::loadLaunchImage(QWidget* pParent, UserSettingsPointer pConfig) const {
    Q_UNUSED(pParent);
    Q_UNUSED(pConfig);
    // TODO: Add support for custom launch image.
    return nullptr;
}

QWidget* QmlSkin::loadSkin(QWidget* pParent,
        UserSettingsPointer pConfig,
        QSet<ControlObject*>* pSkinCreatedControls,
        mixxx::CoreServices* pCoreServices) const {
    Q_UNUSED(pConfig);
    Q_UNUSED(pSkinCreatedControls);
    Q_UNUSED(pCoreServices);
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return nullptr;
    }

    QQuickWidget* pWidget = new QQuickWidget(pParent);
    pWidget->engine()->setBaseUrl(QUrl::fromLocalFile(m_path.absoluteFilePath()));
    pWidget->setSource(QUrl::fromLocalFile(m_path.absoluteFilePath() +
            QStringLiteral("/") + kMainQmlFileName));
    pWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    if (pWidget->status() != QQuickWidget::Ready) {
        qWarning() << "Skin" << name() << "failed to load!";
        return nullptr;
    }
    return pWidget;
}

} // namespace qml
} // namespace skin
} // namespace mixxx

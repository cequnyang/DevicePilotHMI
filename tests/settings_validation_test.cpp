#include <QtTest>

#include "settings/settings_defined.h"
#include "settings/settings_validation.h"

class SettingsValidationTest : public QObject
{
    Q_OBJECT

private slots:
    void validSnapshot();
    void warningTemperatureMustBeLowerThanFault();
    void warningPressureMustBeLowerThanFault();
    void warningTemperatureMustStayInRange();
    void faultTemperatureMustStayInRange();
    void warningPressureMustStayInRange();
    void faultPressureMustStayInRange();
    void updateIntervalMustStayInRange();
};

void SettingsValidationTest::validSnapshot()
{
    Settings::Snapshot s = Settings::defaults();
    auto [ok, reason] = Settings::validateSnapshot(s);

    QVERIFY(ok);
    QVERIFY(reason.isEmpty());
}

void SettingsValidationTest::warningTemperatureMustBeLowerThanFault()
{
    Settings::Snapshot s = Settings::defaults();
    s.warningTemperature = 120;
    s.faultTemperature = 100;

    auto [ok, reason] = Settings::validateSnapshot(s);

    QVERIFY(!ok);
    QVERIFY(!reason.isEmpty());
}

void SettingsValidationTest::warningPressureMustBeLowerThanFault()
{
    Settings::Snapshot s = Settings::defaults();
    s.warningPressure = 120;
    s.faultPressure = 100;

    auto [ok, reason] = Settings::validateSnapshot(s);

    QVERIFY(!ok);
    QVERIFY(!reason.isEmpty());
}

void SettingsValidationTest::warningTemperatureMustStayInRange()
{
    Settings::Snapshot s = Settings::defaults();
    s.warningTemperature = Settings::HighThreshold::kWarningTemperature + 10;

    auto [ok, reason] = Settings::validateSnapshot(s);

    QVERIFY(!ok);
    QVERIFY(!reason.isEmpty());
}

void SettingsValidationTest::faultTemperatureMustStayInRange()
{
    Settings::Snapshot s = Settings::defaults();
    s.faultTemperature = Settings::HighThreshold::kFaultTemperature + 10;

    auto [ok, reason] = Settings::validateSnapshot(s);

    QVERIFY(!ok);
    QVERIFY(!reason.isEmpty());
}

void SettingsValidationTest::warningPressureMustStayInRange()
{
    Settings::Snapshot s = Settings::defaults();
    s.warningPressure = Settings::HighThreshold::kWarningPressure + 10;

    auto [ok, reason] = Settings::validateSnapshot(s);

    QVERIFY(!ok);
    QVERIFY(!reason.isEmpty());
}

void SettingsValidationTest::faultPressureMustStayInRange()
{
    Settings::Snapshot s = Settings::defaults();
    s.faultPressure = Settings::HighThreshold::kFaultPressure + 10;

    auto [ok, reason] = Settings::validateSnapshot(s);

    QVERIFY(!ok);
    QVERIFY(!reason.isEmpty());
}

void SettingsValidationTest::updateIntervalMustStayInRange()
{
    Settings::Snapshot s = Settings::defaults();
    s.updateIntervalMs = Settings::HighThreshold::kUpdateIntervalMs + 10;

    auto [ok, reason] = Settings::validateSnapshot(s);

    QVERIFY(!ok);
    QVERIFY(!reason.isEmpty());
}

QTEST_APPLESS_MAIN(SettingsValidationTest)
#include "settings_validation_test.moc"

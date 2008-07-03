#ifndef REGEXPMATCHERMANAGER_H
#define REGEXPMATCHERMANAGER_H
#include <QAbstractItemModel>
#include <QStringList>
#include <deque>
#include <stdexcept>

class RegexpMatcherManager : public QAbstractItemModel
{
	Q_OBJECT
public:
	typedef std::pair<QString, QString> titlebody_t;
	class AlreadyExists : public std::runtime_error
	{
	public:
		explicit AlreadyExists (const std::string& str)
		: std::runtime_error (str)
		{
		}
	};

	class NotFound : public std::runtime_error
	{
	public:
		explicit NotFound (const std::string& str)
		: std::runtime_error (str)
		{
		}
	};

	class Malformed : public std::runtime_error
	{
	public:
		explicit Malformed (const std::string& str)
		: std::runtime_error (str)
		{
		}
	};

	struct Item
	{
		QString Title_;
		QString Body_;

		Item (const QString& title, const QString& body)
		: Title_ (title)
		, Body_ (body)
		{
		}

		bool operator== (const Item& other) const
		{
			return Title_ == other.Title_ &&
				Body_ == other.Body_;
		}

		bool IsEqual (const QString& str) const
		{
			return Title_ == str;
		}
	};
private:

	QStringList ItemHeaders_;
	typedef std::deque<Item> items_t;
	items_t Items_;

	RegexpMatcherManager ();

	mutable bool SaveScheduled_;
public:
	static RegexpMatcherManager& Instance ();
	virtual ~RegexpMatcherManager ();

	void Release ();
	void Add (const QString&, const QString&);
	void Remove (const QString&);
	void Remove (const QModelIndex&);
	void Modify (const QString&, const QString&);
	titlebody_t GetTitleBody (const QModelIndex&) const;

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
private slots:
	void saveSettings () const;
private:
	void RestoreSettings ();
	void ScheduleSave ();
};

#endif


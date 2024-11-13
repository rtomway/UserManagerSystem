#include "server.h"
#include"SResultCode.h"
#include"SSqlConnectionPool.h"
#include"SHttpPartParse.h"
#include<QFile>
#include<QFileInfo>
#include<QDir>
#include"SJwt.h"
#include<optional>


#define CheckJsonParse(request)\
         QJsonParseError err;\
         auto jdom = QJsonDocument::fromJson(request.body(), &err);\
         if (err.error != QJsonParseError::NoError){\
   return SResult::failure(SResultCode::ParamJsonInvalid);\
         }


#define CheckQueryError(query)\
         if (query.lastError().type() != QSqlError::NoError)\
         {\
                qDebug()<<query.lastError().text();\
         	return SResult::failure(SResultCode::ServerSqlQueryError);\
         }


QJsonObject RecordtoJsonObject(const QSqlRecord & record);

const char* secret = "xu666";

std::optional<QByteArray> CheckToken(const QHttpServerRequest&request)
{
	//校验token
	auto auth = request.value("Authorization");
	if (auth.isEmpty())
	{
		return SResult::failure(SResultCode::UserUnanthorized);
	}
	if (!auth.startsWith("Bearer"))
	{
		return SResult::failure(SResultCode::UserAuthFormatError);
	}

	//拿到token
	auto token = auth.mid(strlen("Bearer"));
	//验证
	auto jwtObject = SJwt::SJwtObject::decode(token, SJwt::SAlgorithm::HS256, secret);
	if (jwtObject.status() == SJwt::SJwtObject::Status::Expired)
	{
		return SResult::failure(SResultCode::UserAuthTokenExpired);
	}

	if (!jwtObject.isValid())
	{
		return SResult::failure(SResultCode::UserAuthTokenInvaild);
	}
	return{};
}


Server::Server()
{
	m_server.listen(QHostAddress::Any, 8888);

	//添加路由丢失处理
	m_server.setMissingHandler([](const QHttpServerRequest& request, QHttpServerResponder&& responder)
		{
			QJsonObject obj;
			obj.insert("code", "3000");
			obj.insert("message", "不支持此操作");
			responder.write(QJsonDocument(obj));
		});
	//版本
	m_server.route("/api/version", [](const QHttpServerRequest& request)
		{
			QJsonObject obj;
			obj.insert("version", "0.0.1");
			obj.insert("name", "UserPrivilegeManager-Server");
			obj.insert("datatime", QDateTime::currentDateTime().toString("yyyy-MM-dd"));
			obj.insert("copyright", "xu");
			return QJsonDocument(obj).toJson(QJsonDocument::Compact);
		});
	//登录
	m_server.route("/api/login", [](const QHttpServerRequest& request)
		{

			CheckJsonParse(request);

			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());
			query.prepare("select * from user where user_id=? and password=? and isDeleted=FALSE limit 1");
			query.bindValue(0, jdom["user_id"].toString());
			query.bindValue(1, jdom["password"].toString());
			if (!query.exec())
			{
				return SResult::failure(SResultCode::ServerSqlQueryError);
			}
			if (!query.next())
			{
				return SResult::failure(SResultCode::UserLoginError);
			}
			//检查用户是否禁用
			if (!query.value("isEnable").toBool())
			{
				return SResult::failure(SResultCode::UserAccountDisable);
			}
			//生成token jwt
			QJsonObject paylocad = {
				{"user_id",jdom["user_id"]},
			    {"iat",QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate)},
				{"exp",QDateTime::currentDateTime().addDays(7).toString(Qt::DateFormat::ISODate)}
			};
			SJwt::SJwtObject jwtObject(SJwt::SAlgorithm::HS256, paylocad, secret);


			return SResult::success({ {"token",QString(jwtObject.jwt())}});
		});
	//注册
	m_server.route("/api/register", [](const QHttpServerRequest& request)
		{
			CheckJsonParse(request);
			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());
			query.prepare("insert ignore into user(user_id,username,password) values(?,?,?)");
			query.bindValue(0, jdom["user_id"].toString());
			query.bindValue(1, jdom["username"].toString());
			query.bindValue(2, jdom["password"].toString());
			query.exec();
			CheckQueryError(query);
			if (query.numRowsAffected() == 0)
			{
				return SResult::failure(SResultCode::UserAccountExists);
			}
			return SResult::success();

		});
	userRoute();
	privilegeRoute();
}

//用户接口
void Server::userRoute()
{
	//用户列表
	m_server.route("/api/user/list", [](const QHttpServerRequest& request)
		{
			std::optional<QByteArray> ret = CheckToken(request);
			if (ret.has_value())
			{
				return ret.value();
			}

			auto uquery = request.query();
			auto user_id = uquery.queryItemValue("user_id");
			auto isEnable = uquery.queryItemValue("isEnable");
			auto page = uquery.queryItemValue("page").toInt();
			auto pageSize = uquery.queryItemValue("pageSize").toInt();
			auto q = uquery.queryItemValue("query");

			QString filter = "where isDeleted=false ";
			if (!user_id.isEmpty())
			{
				filter += QString(" and user_id='%1' ").arg(user_id);
			}
			else if (!q.isEmpty())
			{
				filter += QString(" and user_id like '%%1%' or username like '%%1%' \
					or mobile like '%%1%' or email like '%%1%' ").arg(q);
			}
			if (!isEnable.isEmpty())
			{
				filter += QString(" and isEnable=%1 ").arg(isEnable == "true" ? 1 : 0);
			}


			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());

			//查询总记录条数
			query.exec(QString(" select count(*)  total from user %1 ").arg(filter));
			qDebug() << QString(" select count(*)  total from user %1 ").arg(filter);
			CheckQueryError(query);
			query.next();
			int total = query.value("total").toInt();

			if (pageSize == 0)
				pageSize = 100;
			//获取总页数并判断是否是整页
			int last_page = total / pageSize + (total % pageSize ? 0 : 1) + 1;
			if (page > last_page || page < 1)
			{
				page = 1;
			}


			QString	sql = "select id,user_id,gender,username,mobile,email,isEnable from user ";
			sql += filter;
			sql += QString("limit %1,%2").arg((page - 1) * pageSize).arg(pageSize);

			qDebug() << sql;
			query.exec(sql);
			CheckQueryError(query);

			QJsonObject jobj;

			if (query.size() > 0)
			{
				QJsonArray jarray;
				int count = query.record().count();
				while (query.next())
				{
					auto juser = RecordtoJsonObject(query.record());
					jarray.append(juser);
				}
				jobj.insert("lists", jarray);
				jobj.insert("cur_page", page);
				jobj.insert("pageSize", pageSize);
				jobj.insert("last_page", last_page);
				jobj.insert("total", total);
			}

			return SResult::success(jobj);

		});
	//创建
	m_server.route("/api/user/create", [](const QHttpServerRequest& request)
		{
			std::optional<QByteArray> ret = CheckToken(request);
			if (ret.has_value())
			{
				return ret.value();
			}
			CheckJsonParse(request);
			auto robj = jdom.object();
			if (!(robj.contains("user_id") && robj.contains("username")))
			{
				return SResult::failure(SResultCode::ParamLoss, "缺少user_id或username参数");
			}

			auto password = jdom["password"].toString();
			if (password.isEmpty())
			{
				password = "123456";
			}
			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());
			//user表
			query.prepare("insert ignore into user(user_id, username, gender, mobile, email, password) values(?,?,?,?,?,?)");
			query.bindValue(0, jdom["user_id"].toString());
			query.bindValue(1, jdom["username"].toString());
			query.bindValue(2, jdom["gender"].toInt());

			//null  QVariant()
			query.bindValue(3, jdom["mobile"].toString().isEmpty() ? QVariant() : jdom["mobile"].toString());
			query.bindValue(4, jdom["email"].toString().isEmpty() ? QVariant() : jdom["email"].toString());
			query.bindValue(5, password);
			query.exec();

			//user_privilege表
			query.prepare("insert ignore into user_privilege(user_id, username) values(?,?)");
			query.bindValue(0, jdom["user_id"].toString());
			query.bindValue(1, jdom["username"].toString());
			query.exec();

			CheckQueryError(query);
			if (query.numRowsAffected() == 0)
			{
				return SResult::failure(SResultCode::UserAccountExists);
			}

			return SResult::success();
		});
	//批量创建
	m_server.route("/api/user/batch_create", [](const QHttpServerRequest& request)
		{
			std::optional<QByteArray> ret = CheckToken(request);
			if (ret.has_value())
			{
				return ret.value();
			}
			CheckJsonParse(request);

			SConnectionWrap wrap;
			auto db = wrap.openConnection();
			db.transaction();
			QSqlQuery query(db);
			query.prepare("insert ignore into user(user_id, username, gender, mobile, email, password) values(?,?,?,?,?,?)");

			auto roobj = jdom.object();
			auto jarr = roobj.value("list").toArray();
			for (size_t i = 0; i < jarr.size(); i++)
			{
				auto robj = jarr.at(i).toObject(); qDebug() << robj;
				if (!(robj.contains("user_id") && robj.contains("username")))
				{
					db.rollback();
					return SResult::failure(SResultCode::ParamLoss, "缺少user_id或username参数");
				}
				auto password = robj.value("password").toString();
				if (password.isEmpty())
				{
					password = "123456";
				}
				query.bindValue(0, robj.value("user_id").toString());
				query.bindValue(1, robj.value("username").toString());
				query.bindValue(2, robj.value("gender").toInt());
				//null  QVariant()
				query.bindValue(3, robj.value("mobile").toString().isEmpty() ? QVariant() : robj.value("mobile").toString());
				query.bindValue(4, robj.value("email").toString().isEmpty() ? QVariant() : robj.value("email").toString());
				query.bindValue(5, password);
				if (!query.exec())
				{
					db.rollback();
					return SResult::failure(SResultCode::ServerSqlQueryError);
				}
				//CheckQueryError(query);
			}
			db.commit();

			//是否插入成功
			if (query.numRowsAffected() == 0)
			{
				return SResult::failure(SResultCode::UserAccountExists);
			}

			return SResult::success();
		});
	//删除
	m_server.route("/api/user/delete", [](const QHttpServerRequest& request)
		{
			std::optional<QByteArray> ret = CheckToken(request);
			if (ret.has_value())
			{
				return ret.value();
			}
			CheckJsonParse(request);
			auto jarray = jdom["lists"].toArray();
			if (jarray.isEmpty())
			{
				return SResult::failure(SResultCode::ParamLoss);
			}
			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());
			query.prepare("update user set isDeleted=true where user_id=?");
			for (size_t i = 0; i < jarray.size(); i++)
			{
				query.bindValue(0, jarray.at(i).toString());
				query.exec();
				CheckQueryError(query);
			}
			return SResult::success();
		});
	//修改
	m_server.route("/api/user/alter", [](const QHttpServerRequest& request)
		{
			std::optional<QByteArray> ret = CheckToken(request);
			if (ret.has_value())
			{
				return ret.value();
			}
			auto uquery = request.query();
			if (!uquery.hasQueryItem("user_id"))
			{
				return SResult::failure(SResultCode::ParamLoss);
			}
			CheckJsonParse(request);
			QString update;
			auto robj = jdom.object();
			/*if (robj.contains("username"))
				update += QString("username='%1',").arg(robj.value("username").toString());
			if (robj.contains("gender"))
				update += QString("gender='%1',").arg(robj.value("gender").toInt());
			if (robj.contains("mobile"))
				update += QString("mobile='%1',").arg(robj.value("mobile").toString());
			if (robj.contains("email"))
				update += QString("email='%1',").arg(robj.value("email").toString());
			if (robj.contains("password"))
				update += QString("password='%1',").arg(robj.value("password").toString());
			if (robj.contains("isEnable"))
				update += QString("isEnable='%1',").arg(robj.value("isEnable").toBool());*/

		
				update += QString("username='%1',").arg(robj.value("username").toString());
				update += QString("gender='%1',").arg(robj.value("gender").toInt());
				if (!robj.value("mobile").toString().isEmpty())
					update += QString("mobile='%1',").arg(robj.value("mobile").toString());
				else
					update += QString("mobile=null,");
				if(!robj.value("email").toString().isEmpty())
				update += QString("email='%1',").arg(robj.value("email").toString());
				else
					update += QString("email=null,");
				update += QString("password='%1',").arg(robj.value("password").toString());
				update += QString("isEnable='%1',").arg(robj.value("isEnable").toBool());

			update.chop(1);

			qDebug() << update;

			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());
			query.exec(QString("update user set %1 where user_id='%2'")
				.arg(update).arg(uquery.queryItemValue("user_id")));
			qDebug() <<"66666666666666666666" << QString("update user set %1 where user_id='%2'")
				.arg(update).arg(uquery.queryItemValue("user_id"));
			CheckQueryError(query);
			if (query.numRowsAffected() == 0)
			{
				return SResult::failure(SResultCode::SuccessButNotData);
			}
			return SResult::success();
		});
	//批量修改
	m_server.route("/api/user/batch_alter", [](const QHttpServerRequest& request)
		{
			std::optional<QByteArray> ret = CheckToken(request);
			if (ret.has_value())
			{
				return ret.value();
			}
			CheckJsonParse(request);

			auto jarray = jdom["lists"].toArray();
			if (jarray.isEmpty())
			{
				return SResult::failure(SResultCode::SuccessButNotData);
			}
			QString resuleSql;
			for (size_t i = 0; i < jarray.size(); i++)
			{
				auto jobj = jarray.at(i).toObject();
				//jobj["filter"].toObject();
				auto filterObject = jobj.value("filter").toObject();
				auto updateObject = jobj.value("update").toObject();

				QString filter;
				for (auto it = filterObject.begin(); it != filterObject.end(); it++)
				{
					if (it.key() == ("user_id"))
						filter += QString("user_id='%1' and").arg(it.value().toString());
					else if (it.key() == ("id"))
						filter += QString("id='%1' and").arg(it.value().toInt());
					else if (it.key() == ("isEnable"))
						filter += QString("isEnable='%1' and").arg(it.value().toString());
					else if (it.key() == ("isDelete"))
						filter += QString("isDelete='%1' and").arg(it.value().toString());
				}
				filter.chop(3);

				QString update;
				for (auto it = updateObject.begin(); it != updateObject.end(); it++)
				{
					if (it.key() == ("username"))
						update += QString("username='%1',").arg(it.value().toString());
					else if (it.key() == ("gender"))
						update += QString("gender='%1',").arg(it.value().toInt());
					else if (it.key() == ("mobile"))
						update += QString("mobile='%1',").arg(it.value().toString());
					else if (it.key() == ("email"))
						update += QString("email='%1',").arg(it.value().toString());
					else if (it.key() == ("password"))
						update += QString("password='%1',").arg(it.value().toString());
					else if (it.key() == ("isEnable"))
						update += QString("isEnable='%1',").arg(it.value().toBool());
				}
				update.chop(1);
				auto sql = QString("update user set %1 where %2;")
					.arg(update).arg(filter);
				resuleSql.append(sql);
			}


			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());
			query.exec(resuleSql);
			qDebug() << resuleSql;

			CheckQueryError(query);
			if (query.numRowsAffected() == 0)
			{
				return SResult::failure(SResultCode::SuccessButNotData);
			}
			return SResult::success();
		});

	//用户头像上传 post
	m_server.route("/api/user/avatar", QHttpServerRequest::Method::Post, [](const QHttpServerRequest& request)
		{
			std::optional<QByteArray> ret = CheckToken(request);
			if (ret.has_value())
			{
				return ret.value();
			}
			auto uquery = request.query();
			if (!uquery.hasQueryItem("user_id"))
			{
				return SResult::failure(SResultCode::ParamLoss);
			}
			auto user_id = uquery.queryItemValue("user_id");

			auto data = request.body();

			SHttpPartParse parse(data);
			if (!parse.parse())
			{
				qDebug() << "body parse error";
				return SResult::failure(SResultCode::ParamParseError);
			}
			qDebug() << "xxxxxxxxxx" << parse.name() << parse.fileName();
			auto path = "images/avatar/";
			QDir dir;
			if (!dir.exists(path))
			{
				dir.mkpath(path);
			}
			QFile file(path + user_id + QDateTime::currentDateTime().toString("yyMMddmmhhsszz") + "." + QFileInfo(parse.fileName()).suffix());
			if (!file.open(QIODevice::WriteOnly))
			{
				return SResult::failure(SResultCode::ServerSqlInnerError, "文件保存失败");
			}
			file.write(parse.data());
			file.close();

			//把路径写入数据库
			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());
			query.prepare("update user set avatar_path=?where user_id=? and isDeleted=false");
			query.bindValue(0, file.fileName());
			query.bindValue(1, user_id);
			query.exec();
			CheckQueryError(query);


			if (query.numRowsAffected() == 0)
				return SResult::failure(SResultCode::SuccessButNotData);


			return SResult::success();
		});
	//用户头像获取  get
	m_server.route("/api/user/avatar", QHttpServerRequest::Method::Get, [](const QHttpServerRequest& request, QHttpServerResponder&& responder)
		{
			std::optional<QByteArray> ret = CheckToken(request);
			if (ret.has_value())
			{
				responder.write(ret.value(), "application/json");
				return;
			}
			qDebug() << request.body();
			auto uquery = request.query();
			if (!uquery.hasQueryItem("user_id"))
			{
				responder.write(SResultCode::ParamLoss.toJson(), "application/json");
				return;
			}
			auto user_id = uquery.queryItemValue("user_id");

			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());
			query.prepare("select avatar_path from user where user_id=? and isDeleted=false");
			query.bindValue(0, user_id);
			if (!query.exec())
			{
				responder.write(SResultCode::ServerSqlQueryError.toJson(), "application/json");
				return;
			}

			if (!query.next())
			{
				responder.write(SResultCode::SuccessButNotData.toJson(), "application/json");
				return;
			}
			QFile file(query.value("avatar_path").toString());
			if (!file.open(QIODevice::ReadOnly))
			{
				responder.write(SResult::failure(SResultCode::ServerResourceNotFound, "头像未找到"), "application/json");
				return;
			}
			responder.writeStatusLine();
			responder.writeHeader("Content-Type", QString("image/%1").arg(QFileInfo(file.fileName()).suffix()).toUtf8());
			responder.writeHeader("Content-Length", QByteArray::number(file.size()));
			responder.writeHeader("Content-Disposition", ("attachment;filename=" + QFileInfo(file.fileName()).fileName()).toUtf8());
			responder.writeBody(file.readAll());
		});

}

//权限接口

void Server::privilegeRoute()
{
	//权限列表
	m_server.route("/api/user_privilege/list", [](const QHttpServerRequest& request)
		{
			std::optional<QByteArray> ret = CheckToken(request);
			if (ret.has_value()) 
			{
				return ret.value();
			}

			auto uquery = request.query();
			auto user_id = uquery.queryItemValue("user_id");
			auto page = uquery.queryItemValue("page").toInt();
			auto pageSize = uquery.queryItemValue("pageSize").toInt();
			auto q = uquery.queryItemValue("query");

			QString filter = "where isDeleted=false ";
			if (!user_id.isEmpty())
			{
				filter += QString(" and user_id='%1' ").arg(user_id);
			}
			else if (!q.isEmpty())
			{
				filter += QString(" and user_id like '%%1%' or username like '%%1%' ").arg(q);
			}

			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());

			//查询总记录条数
			query.exec(QString(" select count(*)  total from user_privilege %1 ").arg(filter));
			qDebug() << QString(" select count(*)  total from user_privilege %1 ").arg(filter);
			CheckQueryError(query);
			query.next();
			int total = query.value("total").toInt();

			if (pageSize == 0)
				pageSize = 100;
			//获取总页数并判断是否是整页
			int last_page = total / pageSize + (total % pageSize ? 0 : 1) + 1;
			if (page > last_page || page < 1)
			{
				page = 1;
			}


			QString	sql = "select user_id,username,privilege_read,privilege_edit,privilege_add,privilege_delete,isDeleted from user_privilege ";
			sql += filter;
			sql += QString("limit %1,%2").arg((page - 1) * pageSize).arg(pageSize);

			qDebug() << sql;
			query.exec(sql);
			CheckQueryError(query);

			QJsonObject jobj;
			if (query.size() > 0)
			{
				QJsonArray jarray;
				int count = query.record().count();
				while (query.next())
				{
					auto juser = RecordtoJsonObject(query.record());
					jarray.append(juser);
				}
				jobj.insert("lists", jarray);
				jobj.insert("cur_page", page);
				jobj.insert("pageSize", pageSize);
				jobj.insert("last_page", last_page);
				jobj.insert("total", total);
			}

			return SResult::success(jobj);

		});


	//修改
	m_server.route("/api/user_privilege/alter", [](const QHttpServerRequest& request)
		{
			std::optional<QByteArray> ret = CheckToken(request);
			if (ret.has_value())
			{
				return ret.value();
			}
			auto uquery = request.query();
			if (!uquery.hasQueryItem("user_id"))
			{
				return SResult::failure(SResultCode::ParamLoss);
			}
			CheckJsonParse(request);
			QString update;
			auto robj = jdom.object();
			
			if (robj.contains("privilege_edit"))
				update += QString("privilege_edit='%1',").arg(robj.value("privilege_edit").toBool());
			if (robj.contains("privilege_add"))
				update += QString("privilege_add='%1',").arg(robj.value("privilege_add").toBool());
			if (robj.contains("privilege_delete"))
				update += QString("privilege_delete='%1',").arg(robj.value("privilege_delete").toBool());

			update.chop(1);

			qDebug() << update;

			SConnectionWrap wrap;
			QSqlQuery query(wrap.openConnection());
			query.exec(QString("update user_privilege set %1 where user_id='%2'")
				.arg(update).arg(uquery.queryItemValue("user_id")));

			CheckQueryError(query);
			if (query.numRowsAffected() == 0)
			{
				return SResult::failure(SResultCode::SuccessButNotData);
			}
			return SResult::success();
		});

}

QJsonObject RecordtoJsonObject(const QSqlRecord& record)
{
	QJsonObject juser;
	for (size_t i = 0; i < record.count(); i++)
	{
		const auto& k = record.fieldName(i);
			const auto& v = record.value(i);
			switch (v.typeId())
			{
			case QMetaType::Int:
				juser.insert(k, v.toInt());
				break;
			case QMetaType::Bool:
				juser.insert(k, v.toBool());
				break;
			case QMetaType::UShort:
				juser.insert(k, v.toLongLong());
				break;
			case QMetaType::Float:
				juser.insert(k, v.toDouble());
				break;
			default:
				juser.insert(k, v.toString());
				break;
		}
	}
	return juser;
}

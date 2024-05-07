# Django

## 1. Start a project

- `django-admin startproject xxx`
- 进到子文件夹，改settings.py
  - hosts改成['vcm-38454.vm.duke.edu', '127.0.0.1']
- `python3 manage.py runserver 0:8000`
  - 这时候再点开http://vcm-38454.vm.duke.edu:8000/就有反应了


## 2. Create apps

project = configuration + apps

一个project里面可以有很多app，一个app可以被很多project用

- 在manage.py的同样的directory里，`python manage.py startapp polls`
  - 这样的话就可以当作top-module直接被import，否则在mysite里写的话还要被当作mysite的submodule

### 2.1 Write your first view

- 在app下面的views.py里面加上：

```python
from django.http import HttpResponse

def index(request):
  return HttpResponse("Hello, world. You're at the polls index.")
```

- 为了call这个view函数，在app的directory里面加一个urls.py：

```python
from django.urls import path
from . import views

urlpatterns = [
  path("", views.index, name = "index");
]
```



- `python manage.py check`
  - this checks for any problems in your project without making migrations or touching the database.
- It’s important to add `__str__()` methods to your models, not only for your own convenience when dealing with the interactive prompt, but also because objects’ representations are used throughout Django’s automatically-generated admin.

### three-step guide to making model changes

- Change your models (in `models.py`).
- Run `python manage.py makemigrations`to create migrations for those changes
- Run `python manage.py migrate` to apply those changes to the database.

----------

- In Django, web pages and other content are delivered by views. Each view is represented by a Python function. Django will choose a view by examining the URL that’s requested.
- Each view is responsible for doing one of two things: returning an `HttpResponse` object containing the content for the requested page, or raising an exception such as `Http404`. 

  - All Django wants is that `HttpResponse`. Or an exception.
- By convention `DjangoTemplates` looks for a “templates” subdirectory in each of the `INSTALLED_APPS`
- Since we’re creating a POST form (which can have the effect of modifying data), we need to worry about Cross Site Request Forgeries. Thankfully, you don’t have to worry too hard, because Django comes with a helpful system for protecting against it. In short, all POST forms that are targeted at internal URLs should use the `{% csrf_token %}` template tag.
- (Tutorial 4) Always return an HttpResponseRedirect after successfully dealing with POST data. This prevents data from being posted twice if a user hits the Back button.
- **a common case of basic web development**: getting data from the database according to a parameter passed in the URL, loading a template and returning the rendered template -> shortcut
  - Each generic view needs to know what model it will be acting upon. This is provided using either the `model` attribute (in this example, `model = Question` for `DetailView` and `ResultsView`) or by defining the `get_queryset()` method (as shown in `IndexView`).



- app创建之后要在settings。py里面installed_apps里注册app
- 编写url和views的对应关系
  - 在urls.py里，urlpatterns就是对应的url和函数关系
  - Path("index/", views.index)只要一访问前面的www.xxx.com/index/，就执行后面的views.index这个函数
    - 这么做之前还要在urls.py里面from . import views
- 编写views函数，接收用户请求
  - views.py里面写函数默认需要有一个参数叫request
  - （记得导入httpresponse之类的
- views里面的函数如果想返回html，就不能只返回httpresponse了（这个只返回字符串），而是要return从Django.shortcuts里import来的render
  - Render(request, "xxx.html")
  - xxx.html在app目录下templates文件夹里
- static静态文件：
  - 像图片，js，css，plugins。。。。
  - 得在app目录下创建一个static的文件夹，再在对应view用的html模板里加上引用文件的话
  - 在模板html里面顶部先{% load static %}然后再在对应地方用{% static xxx %}的引用语法
- ORM（Django提供的数据库翻译器）可以帮助我们：
  - 创建，修改，删除数据库中的表（不用写SQL了），但无法创建数据库。数据库得自己创建
  - 操作表中的数据
    - insert, select, delete...
- 数据库：
  - 自带工具创建数据库
  - Django链接数据库
    - 在settings.py里配置和修改DATABASES
- django操作表
  - 创建表
    - 在models.py里
      - from Django.db import models
      - class UserInfo(models.Model)必须继承Model
      - 生成一些field行，然后ORM就会自动翻译成SQL语句
        - 比如name = models.CharField(max_length=32), age = models.IntegerField()
    - 创建完之后，（大前提：此时app需要已经注册，不然这个表不会提交到数据库）python3 manage.py makemigrations，再python3 manage.py migrate
  - 调整表
    - 不想要那个表了的话，把class或者里面的field给注释掉就行。但是在已有的表里，加新field再migrate会有问题（sql会问需不需要默认值）
    - 反正调整表就是在models.py里操作class，再make migration和migrate
- django操作表中的数据(views.py)
  - 新建数据
    - myClass.objects.create(myCharField="xxx", myIntField = 2...)
  - 删除
    - myForm.objects.filter(id=3).delete()
    - myForm.objects.all().delete()
  - 获取
    - myForm.objects.all()返回的是QuerySet类型，返回的是一个全是object的list，这个list里封装了数据库里每一行的内容
    - 所以可以data_list = myForm.objects.all()，再for obj in data_list, print(obj.id, obj.name...)
    - 还可以用filter，但不管筛出来多少个，结果都是object的列表
      - 如果只想要表里第一个obj，可以在最后加一个.first()
  - 更新
    - 用update函数。
      - myForm.objects.all()/filter(xxx...).update(xxField=xxx)



1. urls.py里path写好一个网址，call一个views里还没写的对应函数
2. Views.py里写有对应功能的函数（记得带request参数）
   1. 记得从app的models.py里import涉及到的表！！！就是几个class名
   2. return一个render，render(request, "xx.html")
      1. 如果要给html模板传数据库取来的数据，就要传给render当参数return render(request, "info_list.html", {"data_list": data_list})模板拿到了以后也是可以用来操作的。会转换成真正的html文件再传给用户端
      2. {% for obj in data_list %}
      3. {% endfor %}

- 如果想在页面加一个跳转按钮，可以在对应渲染html里加一个\<a\>href

## 添加用户

- url
- 函数
  - GET，看到页面，输入内容
  - POST，提交->写入到数据库
- 如果想添加完之后跳转别的网页，可以redirect(记得import)到对应url。
  - 如果是别人家的网站，需要域名什么的都写全
  - 如果是自己的网站，则不需要域名，可以直接return redirect("/info/list")



Sudo su - postgres

psql

ALTER USER postgres PASSWORD 'pwd';

\c ridesharingdb

\dt

SELECT \* FROM "myUber_rider";

psql -U postgres -d upsdb
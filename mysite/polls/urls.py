""" hard-way to write views
from unittest import result
from django.urls import path
from . import views

app_name = "polls"
urlpatterns = [
    # ex: /polls/
    path("", views.index, name="index"),
    # ex: /polls/5/
    path("<int:question_id>/", views.detail, name="detail"),
    # ex: /polls/5/results/
    path("<int:question_id>/results/", views.results, name="results"),
    # ex: /polls/5/vote/
    path("<int:question_id>/vote/", views.vote, name="vote"),
]
"""

"""
A common case of basic web development: 
    - getting data from the database according to a parameter passed in the URL
    - loading a template, and 
    - returning the rendered template. 
Because this is so common, Django provides a shortcut, 
called the "generic views" syetem.

Generic views abstract common patterns to the point where you don't even need to 
write Python code to write an app. For example, the ListView and DetailView generic
views abstract the concepts of "display a list of objects" and "display a detail
page for a particular type of object" respectively.
"""

from django.urls import path
from django.views import View
from . import views

app_name = "polls"
urlpatterns = [
    path("", views.IndexView.as_view(), name="index"),
    path("<int:pk>/", views.DetailView.as_view(), name="detail"),
    path("<int:pk>/results/", views.ResultsView.as_view(), name="results"),
    path("<int:question_id>/vote/", views.vote, name="vote"),
]

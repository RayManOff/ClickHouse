Настройки
==========

Описанные в разделе настройки могут быть заданы следующими способами:
* Глобально.
  
  В конфигурационных файлах сервера.

* Для сессии.
  
  При запуске консольного клиента ClickHouse в интерактивном режиме отправьте запрос ``SET setting=value``.

* Для запроса.
  
  * При запуске консольного клиента ClickHouse в неинтерактивном режиме установите параметр запуска ``--setting=value``.
  * При использовании HTTP API передавайте cgi-параметры (``URL?setting_1=value&setting_2=value...``).


Настройки, которые можно задать только в конфигурационном файле сервера, в разделе не рассматриваются.

.. toctree::
    :glob:

    *

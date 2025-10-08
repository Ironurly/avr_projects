class Site
{
    constructor()
    {
        this.kuranty = null;

        $(function () {

            console.log("%c КОШМАР! ", "color:#fff; font-size: 14px; background: #800; padding: 4px 8px;");
            console.log("%c Если вдруг вы захотите ввести TEXT сюда, не вводите, это ошибка! Не используйте дедовский метод.", "color: #888;");

            this.kuranty = {sta: $('#kuranty').data('time'), now: (new Date()).getTime()};
            this.timeTicker();

        }.bind(this));

        // Активация после загрузки страницы
        $(function () {

            // Если на странице есть кодовые блоки, оформить их как требуется
            $('pre.codeblock').each(function () {

                let that = $(this);
                const title = that.is(":visible") ? 'Свернуть код' : 'Развернуть код';
                that.before('<div class="codeblock-toggle">[<span onclick="site.toggleCodeblock(this)">' + title + '</span>]</div>');
            });
        });
    }

    // Отколлапсировать и сколлапсировать код
    toggleCodeblock(that)
    {
        const next = $(that).parent().next('.codeblock');
        $(that).html(next.is(":visible") ? 'Развернуть код' : 'Свернуть код');
        next.slideToggle();
    }

    // Обновление времени на сайте
    timeTicker()
    {
        const uptime = (new Date()).getTime() - this.kuranty.now;
        const update = new Date(this.kuranty.sta + uptime);
        const clocks = ('0' + update.getHours()).substr(-2) + ':' + ('0' + update.getMinutes()).substr(-2);

        $('#kuranty').html(clocks);

        setTimeout(function () { this.timeTicker(); }.bind(this), 1000);
    }

    // Запрос на AJAX
    // https://learn.javascript.ru/xmlhttprequest
    ajax(url, opt = null)
    {
        let xhr     = new XMLHttpRequest();
        let method  = opt && opt.hasOwnProperty('method') ? opt.method : 'GET';
        let async   = opt && opt.hasOwnProperty('async') ? opt.async : true;
        let post    = opt && opt.hasOwnProperty('post') ? opt.post : null;

        if (post) {

            let fd = new FormData();
            for (let n in post) {
                fd.append(n, post[n]);
            }

            xhr.open('POST', url, async);
            xhr.send(fd);

        } else {

            xhr.open(method, url, async);
            xhr.send();
        }

        xhr.onload = function()
        {
            if (opt.hasOwnProperty('done')) {
                opt.done(JSON.parse(xhr.responseText), xhr);
            }

            if (opt.hasOwnProperty('success')) {
                opt.success(xhr.responseText, xhr);
            }
        };

        xhr.onerror = function()
        {
            // alert(`Ошибка соединения`);
        };
    }

    // Отослать AJAX на переключение
    checkbox(that, url = null)
    {
        url = (url ? url : window.location.pathname);

        this.ajax(url, {
            post: {
                id: $(that).data('id'),
                sw: $(that).is(':checked') ? 1 : 0
            }
        })
    }

    // Запрос нотификации
    requestNotify(msg, opt = null)
    {
        if (!("Notification" in window)) {
            return false;
        } else if (Notification.permission === "granted") {
            new Notification(msg, opt)
        } else if (Notification.permission !== "denied") {
            Notification.requestPermission().then((permission) => {
                if (permission === "granted") {
                    new Notification(msg, opt)
                }
            });
        }
    }

    // Табуляция
    tab(selector, event, spaces = 4)
    {
        let that = document.querySelector(selector);
        let v = that.value, s = that.selectionStart, e = that.selectionEnd;

        // Список всех строк до текущего момента, извлечь последнюю строку
        let a = v.substring(0, s).split("\n");
        let u = spaces - (a[a.length - 1].length % spaces);

        // Симуляция пробельной табуляции
        that.value = v.substring(0, s) + (' '.repeat(u)) + v.substring(e);
        that.selectionStart = that.selectionEnd = s + u;

        event.preventDefault();
        event.stopPropagation();
    }

    // Вставить данные в TEXTAREA
    ins(selector, u)
    {
        let that = document.querySelector(selector);
        let v = that.value, s = that.selectionStart, e = that.selectionEnd;
        that.value = v.substring(0, s) + u + v.substring(e);
        that.selectionStart = that.selectionEnd = s + u.length;
    }

    // Добавить в избранное
    addfavorite(that)
    {
        const url   = $(that).data('url');
        const myfav = $('#my-favs');

        ajax("/ajax/favorite", {
            post: {
                url: url,
            },
            done: function (x) {
                if (x.needauth) {
                    window.location = '/profile/login?url=/' + url;
                } else {
                    if (x.type) {
                        myfav.find('i').removeClass('fa-bookmark-o').addClass('fa-bookmark');
                    } else {
                        myfav.find('i').removeClass('fa-bookmark').addClass('fa-bookmark-o');
                    }
                }
            }
        });

        return false;
    }

    // Переключение блоков кода
    switchsub(t)
    {
        const that = $(t);
        let   href = that.attr('href').substring(1);
        href = href ? href : '#';
        const target = that.closest('.codeblock').find('div[data-name="'+href+'"]');

        that.parent().find('a').removeClass('codeblock-active-tab');
        that.addClass('codeblock-active-tab');

        that.closest('.codeblock').find('div[data-name]').addClass('hidden');
        target.removeClass('hidden');

        // Убрать whitespace перед и после через скрипт
        target.html(target.html().trim());

        return false;
    }

    now()
    {
        return (new Date()).getTime();
    }

    save(name, value)
    {
        if (window.localStorage) {
            window.localStorage.setItem(name, value);
        }
    }

    load(name)
    {
        if (window.localStorage) {
            return window.localStorage.getItem(name);
        }
    }
}

// Создание общего объекта класса на все времена
const site = new Site();

// Хелперсы
function ajax(url, opt = null)      { return site.ajax(url, opt); }
function checkbox(that, url = null) { return site.checkbox(that, url); }
function notify(msg, opt = null)    { return site.requestNotify(msg, opt); }
function log()                      { console.log(...arguments); }
function hexb(x, s = 2) { let y = x.toString(16).toUpperCase(); return "0".repeat(s - y.length) + y; }
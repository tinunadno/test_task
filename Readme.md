## Общее описание решения

В данном репозитории есть три ветки:

1.[master](https://github.com/tinunadno/test_task/tree/master) - решение с большим упором на структуру и расширяемость

2.cool_mode_for_console - то-же решение, что и в master, но с более комплексным консольным интерфейсом

3.[simple_solution_branch](https://github.com/tinunadno/test_task/tree/simple_solution_branch) - максимально простое и последовательное решение, где реализация максимально соответствует ТЗ

*описание непосредственно алгоритма и оценки его сложности я приложил в ветке [simple_solution_branch](https://github.com/tinunadno/test_task/tree/simple_solution_branch)*

## Описание решения

**coolmode доступен только на unix-подобных систем и потребуется установить ncurse: *sudo apt-get install libncurses5-dev libncursesw5-dev***


Общая структура проекта такая же, как и в [master](https://github.com/tinunadno/test_task/tree/master), за тем исключением, что тут 
я добавил *coolmode* при входе в который отображается карта домов и станций и отрисовываются связи между объектами первого и второго типа
 и есть возможность *подсветить* нужные связи, например между объектом второго типа и всеми привязанными объектами первого типа, или показать связь объекта первого типа со вторым, пример работы:

### пример работы *coolmode*

![example](content/example.gif)

**качество гифки низкое из-за конвертации**
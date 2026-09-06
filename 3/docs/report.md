**Отчет по лабораторной работе 2**

**гр. 451001, Хренков Даниил**

Добавлен класс реестр типов продуктов ProductFactory. Каждый тип регистрируется по строковому имени (например \"Ноутбук\", \"Смартфон\", \"Наушники\"). Для типа хранится функция-создатель, которая принимает набор параметров в виде ProductParams (словарь ключ-значение) и массив FieldSpec с описанием дополнительных полей для ввода (тип поля: String, Int, Double, Bool; подписи).

Параметры передаются в фабрику как ProductParams: обязательно name, price, manufacturer, country, остальные ключи задаются в FieldSpec (warranty, ram, storage и т.д.).

В файлах Laptop.cs, Smartphone.cs, Headphones.cs добавлена саморегистрация: при загрузке программы через атрибут \[ModuleInitializer\] вызывается RegisterType с именем типа, лямбдой и массивом FieldSpec. Так новый класс можно добавить, не меняя Program.cs и саму фабрику.

Program.cs: AddProduct и ShowByType используют factory.AvailableTypes(), factory.FieldsFor(type) и factory.Create(typeName, params). ShowCategories использует рефлексию для автоматического обнаружения всех классов-наследников Product.

**Как работает:**

1\) При старте программы срабатывают все \[ModuleInitializer\]-методы и каждый тип регистрируется в \_types (Dictionary: имя типа -\> создатель + поля).

2\) AvailableTypes() отдает список зарегистрированных имен для динамического построения меню.

3\) FieldsFor(type) отдает массив дополнительных полей; Program.cs спрашивает их у пользователя и кладет в ProductParams.

4\) Create(type, params) находит создателя и возвращает Product.

**Файл Factories/FieldSpec.cs:**

public enum FieldType { String, Int, Double, Bool }\
public record FieldSpec(string Key, string Label, FieldType Type);

**Файл Factories/ProductParams.cs:**

public class ProductParams\
{\
private readonly Dictionary\<string, object\> \_data = new();\
public void Set(string key, object value) =\> \_data\[key\] = value;\
public T Get\<T\>(string key) =\> (T)Convert.ChangeType(\_data\[key\], typeof(T));\
}

**Файл Factories/ProductFactory.cs:**

public class ProductFactory\
{\
public static ProductFactory Instance { get; } = new();\
private readonly Dictionary\<string, TypeEntry\> \_types = new();\
private record TypeEntry(Func\<ProductParams, Product\> Creator, FieldSpec\[\] Fields);\
\
public bool RegisterType(string typeName, Func\<ProductParams, Product\> creator, FieldSpec\[\] fields)\
{\
return \_types.TryAdd(typeName, new TypeEntry(creator, fields));\
}\
public List\<string\> AvailableTypes() =\> \_types.Keys.ToList();\
public FieldSpec\[\] FieldsFor(string typeName) =\> \_types\[typeName\].Fields;\
public Product Create(string typeName, ProductParams p) =\> \_types\[typeName\].Creator(p);\
}

**Пример саморегистрации (Laptop.cs, добавлен в конец файла):**

internal static class LaptopRegistration\
{\
\[ModuleInitializer\]\
public static void Register()\
{\
ProductFactory.Instance.RegisterType(\"Ноутбук\",\
p =\> new Laptop(\
p.Get\<string\>(\"name\"), p.Get\<decimal\>(\"price\"),\
new Manufacturer(p.Get\<string\>(\"manufacturer\"), p.Get\<string\>(\"country\")),\
p.Get\<int\>(\"warranty\"), p.Get\<double\>(\"power\"),\
p.Get\<int\>(\"ram\"), p.Get\<double\>(\"screen\"),\
p.Get\<bool\>(\"touch\")),\
\[\
new(\"warranty\", \"Гарантия (мес.)\", FieldType.Int),\
new(\"power\", \"Мощность (W)\", FieldType.Double),\
new(\"ram\", \"ОЗУ (GB)\", FieldType.Int),\
new(\"screen\", \"Диагональ экрана (дюймы)\", FieldType.Double),\
new(\"touch\", \"Сенсорный экран\", FieldType.Bool),\
\]);\
}\
}

**Program.cs: AddProduct через фабрику:**

var types = ProductFactory.Instance.AvailableTypes();\
for (int i = 0; i \< types.Count; i++)\
Console.WriteLine(\$\" {i + 1}. {types\[i\]}\");\
// \...\
var p = new ProductParams();\
p.Set(\"name\", name);\
p.Set(\"price\", price);\
p.Set(\"manufacturer\", mfName);\
p.Set(\"country\", mfCountry);\
\
foreach (var field in ProductFactory.Instance.FieldsFor(typeName))\
{\
// ввод значения по field.Type и field.Label\
p.Set(field.Key, value);\
}\
catalog.Add(ProductFactory.Instance.Create(typeName, p));

**Program.cs: ShowByType через фабрику и рефлексию:**

var types = ProductFactory.Instance.AvailableTypes();\
// динамическое меню из зарегистрированных типов\
string selectedCategory = types\[choice - 1\];\
var productType = Assembly.GetExecutingAssembly().GetTypes()\
.FirstOrDefault(t =\> t.GetCustomAttribute\<ProductCategoryAttribute\>()?.Category == selectedCategory\
&& !t.IsAbstract);\
var filtered = catalog.GetAll().Where(p =\> productType.IsInstanceOfType(p)).ToList();

**Program.cs: ShowCategories через рефлексию:**

var types = Assembly.GetExecutingAssembly().GetTypes()\
.Where(t =\> typeof(Product).IsAssignableFrom(t) && t != typeof(Product));\
foreach (var type in types)\
{\
var attr = type.GetCustomAttribute\<ProductCategoryAttribute\>();\
Console.WriteLine(\$\" Класс: {type.Name,-20} Категория: {attr?.Category ?? \\\"\[атрибут не задан\]\\\"}\");\
}

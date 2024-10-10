#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "Collection.h"
#include "Stream.h"

#ifndef __PDFOBJECT
#define __PDFOBJECT

#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Text;

struct pdf_obj {
	short refs;
	unsigned char kind;
	unsigned char flags;
};


namespace MuPDF {

public enum class Kind {
	Null,
	Boolean,
	Name,
	Integer,
	Float,
	String,
	Array,
	Dictionary,
	Reference,
	Stream,
	Unknown
};

/// <summary>
/// Encapsulates the PDF obj used by MuPDF
/// </summary>
public ref class PdfObject : IDisposable, IEquatable<PdfObject^> {
public:
	/// <summary>
	/// Gets or sets whether this object is dirty (modified).
	/// </summary>
	property bool IsDirty {
		bool get() { return pdf_obj_is_dirty(_ctx, _obj); }
		void set(bool value) {
			if (value) {
				pdf_dirty_obj(_ctx, _obj);
			}
			else {
				pdf_clean_obj(_ctx, _obj);
			}
		}
	}
	/// <summary>
	/// Provides direct object kind info.
	/// </summary>
	property Kind TypeKind {
		virtual Kind get();
	}
	property bool IsIndirect {
		bool get() { return pdf_is_indirect(_ctx, _obj); }
	}
	property bool IsStream {
		bool get() { return pdf_is_stream(_ctx, _obj); }
	}
	property bool IsName {
		bool get() { return pdf_is_name(_ctx, _obj); }
	}
	property bool IsNull {
		bool get() { return pdf_is_null(_ctx, _obj); }
	}
	property bool IsBoolean {
		bool get() { return pdf_is_bool(_ctx, _obj); }
	}
	property bool IsInteger {
		bool get() { return pdf_is_int(_ctx, _obj); }
	}
	property bool IsFloat {
		bool get() { return pdf_is_real(_ctx, _obj); }
	}
	property bool IsNumber {
		bool get() { return pdf_is_number(_ctx, _obj); }
	}
	property bool IsString {
		bool get() { return pdf_is_string(_ctx, _obj); }
	}
	property bool IsArray {
		bool get() { return pdf_is_array(_ctx, _obj); }
	}
	property bool IsDictionary {
		bool get() { return pdf_is_dict(_ctx, _obj); }
	}
	property bool IsPredefined {
		bool get() { return _obj < PDF_LIMIT; }
	}
	property int IntegerValue {
		int get() { return pdf_to_int(_ctx, _obj); }
	}
	property int LongValue {
		int get() { return pdf_to_int64(_ctx, _obj); }
	}
	property float FloatValue {
		float get() { return pdf_to_real(_ctx, _obj); }
	}
	property PdfObject^ UnderlyingObject {
		PdfObject^ get() {
			return pdf_is_indirect(_ctx, _obj)
				? Wrap(pdf_resolve_indirect_chain(_ctx, _obj))
				: this;
		}
	}
	void MarkDirty() {
		pdf_dirty_obj(_ctx, _obj);
	}

	static bool operator == (PdfObject^ x, PdfObject^ y) {
		return Object::ReferenceEquals(x, y) || x && y && pdf_objcmp(x->_ctx, x->Ptr, y->Ptr) == 0;
	}
	static bool operator != (PdfObject^ x, PdfObject^ y) {
		return !Object::ReferenceEquals(x, y) && x && y && pdf_objcmp(x->_ctx, x->Ptr, y->Ptr) != 0;
	}
	virtual bool Equals(PdfObject^ other);
	virtual bool Equals(Object^ obj) override {
		PdfObject^ p;
		return (p = dynamic_cast<PdfObject^>(obj)) && _obj == p->_obj;
	}
	virtual int GetHashCode() override {
		return (int)_obj;
	}
	virtual String^ ToString() override {
		return TypeKind.ToString();
	}
internal:
	PdfObject(pdf_obj* obj) : _obj(obj) {
		if (obj >= PDF_LIMIT) {
			pdf_keep_obj(_ctx = Context::Ptr, obj);
		}
	}
	~PdfObject() {
		ReleaseHandle();
	}
	!PdfObject() {
		ReleaseHandle();
	}
	property fz_context* Ctx { fz_context* get() { return _ctx; } }
	property pdf_obj* Ptr { pdf_obj* get() { return _obj; } }
	static PdfObject^ Wrap(pdf_obj* obj);
private:
	pdf_obj* _obj;
	fz_context* _ctx;

	void ReleaseHandle() {
		if (_obj && _ctx) {
			pdf_drop_obj(_ctx, _obj);
			_obj = NULL;
			_ctx = NULL;
		}
	}
};

public ref class PdfNull : PdfObject {
public:
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Null; }
	}
	static const PdfNull^ Instance = gcnew PdfNull();
	virtual String^ ToString() override { return "<null>"; }
internal:
	PdfNull() : PdfObject(PDF_NULL) {}
};

public ref class PdfBoolean : PdfObject {
public:
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Boolean; }
	}
	static const PdfBoolean^ True = gcnew PdfBoolean(true);
	static const PdfBoolean^ False = gcnew PdfBoolean(false);
	property bool Value {
		bool get() { return Ptr == PDF_TRUE; }
	}
	virtual String^ ToString() override { return Ptr == PDF_TRUE ? "<true>" : "<false>"; }
internal:
	PdfBoolean(bool value) : PdfObject(value ? PDF_TRUE : PDF_FALSE) {}
};

#define PDF_MAKE_NAME(STRING,NAME) NAME,
public enum class PdfNames {
	Undefined,
	True,
	False,
#include "name-table.h"
	AllPredefinedNames
};

public ref class PdfName : PdfObject {
public:
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Name; }
	}
	property String^ Name {
		String^ get() { return gcnew String(pdf_to_name(Context::Ptr, Ptr)); }
	}
	static operator PdfName^(PdfNames value) {
		return gcnew PdfName((pdf_obj*)value);
	}
	virtual String^ ToString() override { return "/" + Name; }
internal:
	PdfName(pdf_obj* obj) : PdfObject(obj) {};
};

public ref class PdfInteger : PdfObject {
public:
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Integer; }
	}
	property int Value {
		int get() { return pdf_to_int(Context::Ptr, Ptr); }
		void set(int value) { pdf_set_int(Context::Ptr, Ptr, value); }
	}
	property long LongValue {
		long get() { return pdf_to_int64(Context::Ptr, Ptr); }
		void set(long value) { pdf_set_int(Context::Ptr, Ptr, value); }
	}
	virtual String^ ToString() override { return LongValue.ToString(); }
internal:
	PdfInteger(pdf_obj* obj) : PdfObject(obj) {};
};

public ref class PdfFloat : PdfObject {
public:
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Float; }
	}
	property float Value {
		float get() { return pdf_to_real(Context::Ptr, Ptr); }
	}
	virtual String^ ToString() override { return Value.ToString(); }
internal:
	PdfFloat(pdf_obj* obj) : PdfObject(obj) {};
};

public ref class PdfString : PdfObject {
public:
	property Kind TypeKind {
		virtual Kind get() override { return Kind::String; }
	}
	property String^ Value {
		String^ get();
	}
	property int Length {
		int get() { return pdf_to_str_len(Context::Ptr, Ptr); }
	}
	/// <summary>
	/// Gets underlying bytes in a PDF string.
	/// </summary>
	array<Byte>^ GetBytes();
	virtual String^ ToString() override { return Value; }
internal:
	PdfString(pdf_obj* obj) : PdfObject(obj) {};
private:
	String^ _string;
	String^ DecodePdfString();
};

public ref class PdfContainer abstract : PdfObject {
public:
	property int Count {
		virtual int get() abstract;
	}
protected:
	pdf_obj* NewPdfString(String^ text);
	PdfContainer(pdf_obj* obj) : PdfObject(obj) {};
private:
	static Encoding^ AsciiEncoding = Encoding::ASCII;
};

public ref class PdfDictionary : PdfContainer, System::Collections::Generic::IEnumerable<KeyValuePair<PdfName^, PdfObject^>>, IIndexableCollection<KeyValuePair<PdfName^, PdfObject^>> {
public:
	property int Count {
		virtual int get() override { return pdf_dict_len(Context::Ptr, Ptr); }
	}
	property PdfName^ Type {
		PdfName^ get() {
			auto o = pdf_dict_get(Context::Ptr, Ptr, PDF_NAME(Type));
			return pdf_is_name(Context::Ptr, o) ? gcnew PdfName(o) : nullptr;
		}
	}
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Dictionary; }
	}
	PdfName^ GetKey(int index) {
		return gcnew PdfName(pdf_dict_get_key(Context::Ptr, Ptr, index));
	}
	PdfObject^ GetValue(int index) {
		return PdfObject::Wrap(pdf_dict_get_val(Context::Ptr, Ptr, index));
	}
	PdfObject^ GetValue(PdfNames key) {
		return PdfObject::Wrap(pdf_dict_get(Context::Ptr, Ptr, (pdf_obj*)key));
	}
	PdfObject^ GetValue(PdfNames key, PdfNames abbrev) {
		return PdfObject::Wrap(pdf_dict_geta(Context::Ptr, Ptr, (pdf_obj*)key, (pdf_obj*)abbrev));
	}
	PdfObject^ InheritableGet(PdfNames key) {
		return PdfObject::Wrap(pdf_dict_get_inheritable(Context::Ptr, Ptr, (pdf_obj*)key));
	}
	PdfObject^ Locate(... array<PdfNames>^ names);
	void Set(PdfNames key, PdfNames value) {
		pdf_dict_put_drop(Context::Ptr, Ptr, (pdf_obj*)key, (pdf_obj*)value);
	}
	void Set(PdfNames key, String^ value) {
		pdf_dict_put_drop(Context::Ptr, Ptr, (pdf_obj*)key, NewPdfString(value));
	}
	void Set(PdfNames key, DateTime dateTime) {
		pdf_dict_put_date(Context::Ptr, Ptr, (pdf_obj*)key, dateTime.ToUniversalTime().Subtract(DateTime(1970, 1, 1)).TotalSeconds);
	}
	void Sort() {
		pdf_sort_dict(Context::Ptr, Ptr);
	}
	bool Remove(PdfNames key) {
		int i = pdf_dict_len(Context::Ptr, Ptr);
		pdf_dict_del(Context::Ptr, Ptr, (pdf_obj*)key);
		return i != pdf_dict_len(Context::Ptr, Ptr);
	}
	PdfDictionary^ DeepClone() {
		return gcnew PdfDictionary(pdf_deep_copy_obj(Context::Ptr, Ptr));
	}
	virtual String^ ToString() override {
		PdfName^ type = Type;
		return type ? String::Concat("{", type->ToString(), "}") : "{}";
	}
	virtual System::Collections::Generic::IEnumerator<KeyValuePair<PdfName^, PdfObject^>>^ GetEnumerator() sealed = System::Collections::Generic::IEnumerable<KeyValuePair<PdfName^, PdfObject^>>::GetEnumerator {
		return gcnew IndexableEnumerator<PdfDictionary^, KeyValuePair<PdfName^, PdfObject^>>(this);
	}
	virtual System::Collections::IEnumerator^ GetEnumeratorBase() sealed =
		System::Collections::IEnumerable::GetEnumerator {
		return GetEnumerator();
	}
internal:
	PdfDictionary(pdf_obj* obj) : PdfContainer(obj) {};
public:
	property KeyValuePair<PdfName^, PdfObject^> default[int] {
		virtual KeyValuePair<PdfName^, PdfObject^> get(int index) {
			return KeyValuePair<PdfName^, PdfObject^>(GetKey(index), GetValue(index));
		};
	}
	property PdfObject^ default[PdfNames] {
		PdfObject^ get(PdfNames key) { return PdfObject::Wrap(pdf_dict_get(Context::Ptr, Ptr, (pdf_obj*)key)); }
		void set(PdfNames key, PdfObject^ value) { pdf_dict_put(Context::Ptr, Ptr, (pdf_obj*)key, value->Ptr); }
	}
	property PdfObject^ default[PdfName^] {
		PdfObject^ get(PdfName^ key) { return PdfObject::Wrap(pdf_dict_get(Context::Ptr, Ptr, key->Ptr)); }
		void set(PdfName^ key, PdfObject^ value) { pdf_dict_put(Context::Ptr, Ptr, key->Ptr, value->Ptr); }
	}
};

public ref class PdfStream : PdfDictionary {
public:
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Stream; }
	}
	Stream^ Open() {
		return gcnew Stream(pdf_open_stream(Context::Ptr, Ptr));
	}
	Stream^ OpenRaw() {
		return gcnew Stream(pdf_open_raw_stream(Context::Ptr, Ptr));
	}
	array<Byte>^ GetBytes() {
		Stream^ s = Open();
		try {
			return s->ReadAll();
		}
		finally {
			delete s;
		}
	}
	array<Byte>^ GetRawBytes() {
		Stream^ s = OpenRaw();
		try {
			return s->ReadAll();
		}
		finally {
			delete s;
		}
	}

	/// <summary>
	/// Replaces bytes in the stream. The data must match /Filter, if <paramref name="compress"/> is true.
	/// </summary>
	/// <param name="data">The data to be placed into the stream.</param>
	/// <param name="compress">Whether the data is compressed. If not compressed, /Filter and /DecodeParms will be removed.</param>
	void SetBytes(array<Byte>^ data, bool compress);
internal:
	PdfStream(pdf_obj* obj) : PdfDictionary(obj) {};
};

public ref class PdfDocumentInfo : PdfDictionary {
public:
	property String^ Title {
		String^ get() { return GetString(PdfNames::Title); }
	}
	property String^ Subject {
		String^ get() { return GetString(PdfNames::Subject); }
	}
	property String^ Producer {
		String^ get() { return GetString(PdfNames::Producer); }
	}
	property String^ Creator {
		String^ get() { return GetString(PdfNames::Creator); }
	}
	property String^ Author {
		String^ get() { return GetString(PdfNames::Author); }
	}
	property String^ Keywords {
		String^ get() { return GetString(PdfNames::Keywords); }
	}
	property String^ CreationDate {
		String^ get() { return GetString(PdfNames::CreationDate); }
	}
	property String^ ModificationDate {
		String^ get() { return GetString(PdfNames::ModDate); }
	}
internal:
	PdfDocumentInfo(pdf_obj* obj) : PdfDictionary(obj) {};
private:
	String^ GetString(PdfNames key) {
		return GetValue(key)->ToString();
	}
};

public ref class PdfArray : PdfContainer, System::Collections::Generic::IEnumerable<PdfObject^>, IIndexableCollection<PdfObject^> {
public:
	property int Count {
		virtual int get() override { return pdf_array_len(Context::Ptr, Ptr); }
	}
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Array; }
	}
	PdfObject^ Get(int index) {
		return Wrap(pdf_array_get(Context::Ptr, Ptr, index));
	}
	bool Contains(PdfObject^ obj) {
		return pdf_array_contains(Context::Ptr, Ptr, obj->Ptr);
	}
	int IndexOf(PdfObject^ obj) {
		return pdf_array_find(Context::Ptr, Ptr, obj->Ptr);
	}
	void Append(bool value) {
		pdf_array_push_bool(Context::Ptr, Ptr, value);
	}
	void Append(long value) {
		pdf_array_push_int(Context::Ptr, Ptr, value);
	}
	void Append(double value) {
		pdf_array_push_real(Context::Ptr, Ptr, value);
	}
	void Append(PdfNames value) {
		pdf_array_push_drop(Context::Ptr, Ptr, (pdf_obj*)value);
	}
	void Append(String^ value) {
		pdf_array_push_drop(Context::Ptr, Ptr, NewPdfString(value));
	}
	void Append(PdfObject^ value) {
		pdf_array_push_drop(Context::Ptr, Ptr, value->Ptr);
	}
	void Set(int index, bool value) {
		pdf_array_put_bool(Context::Ptr, Ptr, index, value);
	}
	void Set(int index, long value) {
		pdf_array_put_int(Context::Ptr, Ptr, index, value);
	}
	void Set(int index, double value) {
		pdf_array_put_real(Context::Ptr, Ptr, index, value);
	}
	void Set(int index, PdfNames value) {
		pdf_array_put(Context::Ptr, Ptr, index, (pdf_obj*)value);
	}
	void Set(int index, String^ value) {
		pdf_array_put_drop(Context::Ptr, Ptr, index, NewPdfString(value));
	}
	void Set(int index, PdfObject^ value) {
		pdf_array_put_drop(Context::Ptr, Ptr, index, value->Ptr);
	}
	void InsertAt(int index, PdfObject^ value) {
		pdf_array_insert_drop(Context::Ptr, Ptr, value->Ptr, index);
	}
	void RemoveAt(int index) {
		pdf_array_delete(Context::Ptr, Ptr, index);
	}
	PdfArray^ DeepClone() {
		return gcnew PdfArray(pdf_deep_copy_obj(Context::Ptr, Ptr));
	}
	virtual String^ ToString() override { return String::Concat("[", Count.ToString(), "]"); }
	virtual System::Collections::Generic::IEnumerator<PdfObject^>^ GetEnumerator() sealed = System::Collections::Generic::IEnumerable<PdfObject^>::GetEnumerator{
		return gcnew IndexableEnumerator<PdfArray^, PdfObject^>(this);
	}
	virtual System::Collections::IEnumerator^ GetEnumeratorBase() sealed = System::Collections::IEnumerable::GetEnumerator {
		return GetEnumerator();
	}
internal:
	PdfArray(pdf_obj* obj) : PdfContainer(obj) {};
public:
	property PdfObject^ default[int] {
		virtual PdfObject^ get(int index) { return Wrap(pdf_array_get(Context::Ptr, Ptr, index)); }
		void set(int index, PdfObject^ value) { pdf_array_put_drop(Context::Ptr, Ptr, index, value->Ptr); }
	}
};

public ref class PdfReference : PdfObject {
public:
	property int Number {
		int get() { return pdf_to_num(Context::Ptr, Ptr); }
	}
	property int Generation {
		int get() { return pdf_to_gen(Context::Ptr, Ptr); }
	}
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Reference; }
	}
	PdfObject^ Resolve() {
		return Wrap(pdf_resolve_indirect_chain(Context::Ptr, Ptr));
	}
	virtual String^ ToString() override {
		return Number.ToString() + " " + Generation.ToString() + " R";
	}
internal:
	PdfReference(pdf_obj* obj) : PdfObject(obj) {};
};

};

#endif // !__PDFOBJECT
